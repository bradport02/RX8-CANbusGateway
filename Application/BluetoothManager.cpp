#include "BluetoothManager.h"

#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QSettings>
#include <QDebug>
#include <QDBusArgument>
#include <QDBusVariant>

// Type aliases and Q_DECLARE_METATYPE are in BluetoothManager.h

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<InterfaceMap>();
    qDBusRegisterMetaType<ManagedObjectsMap>();
    qDBusRegisterMetaType<QMap<QString, QDBusVariant>>();

    QDBusConnection bus = QDBusConnection::systemBus();

    m_adapter = new QDBusInterface(
        "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", bus, this);

    if (!m_adapter->isValid()) {
        qWarning() << "[BT] BlueZ adapter not available:" << m_adapter->lastError().message();
        emit errorOccurred("Bluetooth adapter not found");
        return;
    }

    setupSignals();
    refreshPairedDevices();
}

void BluetoothManager::setupSignals()
{
    QDBusConnection bus = QDBusConnection::systemBus();

    bus.connect("org.bluez", "/",
                "org.freedesktop.DBus.ObjectManager", "InterfacesAdded",
                this, SLOT(onInterfacesAdded(QDBusObjectPath, QMap<QString,QVariantMap>)));

    bus.connect("org.bluez", "/",
                "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved",
                this, SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));

    bus.connect("org.bluez", "/org/bluez/hci0",
                "org.freedesktop.DBus.Properties", "PropertiesChanged",
                this, SLOT(onAdapterPropertiesChanged(QString, QVariantMap, QStringList)));
}

void BluetoothManager::watchDeviceProperties(const QString &dbusPath)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    bus.connect("org.bluez", dbusPath,
                "org.freedesktop.DBus.Properties", "PropertiesChanged",
                this, SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));
}

bool BluetoothManager::isPowered() const
{
    if (!m_adapter || !m_adapter->isValid()) return false;
    return m_adapter->property("Powered").toBool();
}

void BluetoothManager::setPower(bool on)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface props("org.bluez", "/org/bluez/hci0",
                         "org.freedesktop.DBus.Properties", bus);
    props.call("Set", "org.bluez.Adapter1", "Powered",
               QVariant::fromValue(QDBusVariant(on)));
}

void BluetoothManager::startDiscovery()
{
    if (!m_adapter || !m_adapter->isValid()) return;

    // BlueZ 5.56+ requires SetDiscoveryFilter before StartDiscovery
    // Using empty filter = discover all transport types
    QDBusInterface adapter("org.bluez", "/org/bluez/hci0",
                           "org.bluez.Adapter1", QDBusConnection::systemBus());

    QVariantMap filter;
    filter["Transport"] = QString("auto");
    adapter.call("SetDiscoveryFilter", filter);

    QDBusMessage reply = m_adapter->call("StartDiscovery");
    if (reply.type() == QDBusMessage::ErrorMessage)
        qWarning() << "[BT] StartDiscovery failed:" << reply.errorMessage();
    else
        qDebug() << "[BT] Discovery started";
}

void BluetoothManager::stopDiscovery()
{
    if (!m_adapter || !m_adapter->isValid()) return;
    QDBusMessage reply = m_adapter->call("StopDiscovery");
    if (reply.type() == QDBusMessage::ErrorMessage)
        qWarning() << "[BT] StopDiscovery failed:" << reply.errorMessage();
}

// Internal helper — performs the actual pair/connect without any disconnect logic
void BluetoothManager::doConnect(const QString &address)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QString path = devicePath(address);
    QDBusInterface device("org.bluez", path, "org.bluez.Device1", bus);

    if (!device.isValid()) {
        emit errorOccurred("Device not found: " + address);
        return;
    }

    bool alreadyPaired = device.property("Paired").toBool();

    if (!alreadyPaired) {
        qDebug() << "[BT] Device not paired, pairing first:" << address;
        auto *watcher = new QDBusPendingCallWatcher(
            device.asyncCall("Pair"), this);

        connect(watcher, &QDBusPendingCallWatcher::finished,
                this, [this, address, watcher]() {
                    watcher->deleteLater();
                    QDBusPendingReply<> reply = *watcher;
                    if (reply.isError()) {
                        qWarning() << "[BT] Pair failed:" << reply.error().message();
                        emit errorOccurred("Pairing failed: " + reply.error().message());
                        return;
                    }
                    qDebug() << "[BT] Paired OK, trusting and connecting:" << address;

                    QString devPath = devicePath(address);
                    QDBusInterface props("org.bluez", devPath,
                                         "org.freedesktop.DBus.Properties",
                                         QDBusConnection::systemBus());
                    props.call("Set", "org.bluez.Device1", "Trusted",
                               QVariant::fromValue(QDBusVariant(true)));

                    refreshPairedDevices();
                    QDBusInterface dev("org.bluez", devPath,
                                       "org.bluez.Device1", QDBusConnection::systemBus());
                    dev.asyncCall("Connect");
                    QSettings settings("headunit", "bluetooth");
                    settings.setValue("lastDevice", address);
                });
    } else {
        qDebug() << "[BT] Already paired, connecting:" << address;

        bool alreadyConnected = device.property("Connected").toBool();
        if (alreadyConnected) {
            qDebug() << "[BT] Device already connected, emitting connectedChanged";
            refreshPairedDevices();
            emit connectedChanged();
        } else {
            device.asyncCall("Connect");
        }

        QSettings settings("headunit", "bluetooth");
        settings.setValue("lastDevice", address);
    }
}

void BluetoothManager::connectDevice(const QString &address)
{
    if (m_switching) {
        qDebug() << "[BT] Already switching, ignoring connect request for" << address;
        return;
    }

    // ── Single-connection policy ─────────────────────────────────────────────
    if (!m_connectedAddress.isEmpty() && m_connectedAddress != address) {
        qDebug() << "[BT] Switching from" << m_connectedAddress << "to" << address;

        m_switching = true;            // block signals + re-entry during switch
        QString switchingFrom = m_connectedAddress;
        m_connectedAddress.clear();
        QDBusInterface oldDev("org.bluez", devicePath(switchingFrom),
                              "org.bluez.Device1", QDBusConnection::systemBus());
        oldDev.asyncCall("Disconnect");

        QTimer::singleShot(1000, this, [this, address]() {
            m_switching = false;
            doConnect(address);
        });
        return;
    }

    doConnect(address);
}


void BluetoothManager::disconnectDevice(const QString &address)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface device("org.bluez", devicePath(address), "org.bluez.Device1", bus);
    device.asyncCall("Disconnect");
}

void BluetoothManager::removeDevice(const QString &address)
{
    if (address.isEmpty()) {
        qWarning() << "[BT] removeDevice called with empty address — ignoring";
        return;
    }

    QString path = devicePath(address);
    qDebug() << "[BT] Removing device:" << address << "path:" << path;

    // BlueZ requires the device to be disconnected before removal.
    // Disconnect synchronously first, ignore errors (may already be disconnected).
    QDBusInterface device("org.bluez", path, "org.bluez.Device1",
                          QDBusConnection::systemBus());
    if (device.isValid()) {
        bool isConnected = device.property("Connected").toBool();
        if (isConnected) {
            qDebug() << "[BT] Disconnecting before removal...";
            QDBusMessage dcReply = device.call("Disconnect");
            if (dcReply.type() == QDBusMessage::ErrorMessage)
                qWarning() << "[BT] Disconnect failed (continuing anyway):" << dcReply.errorMessage();
            // Disconnect is async — BlueZ will process it before RemoveDevice
        }
    }

    QDBusMessage reply = m_adapter->call(
        "RemoveDevice", QVariant::fromValue(QDBusObjectPath(path)));

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "[BT] RemoveDevice failed:" << reply.errorMessage();
        emit errorOccurred("Failed to remove device: " + reply.errorMessage());
    } else {
        qDebug() << "[BT] RemoveDevice succeeded";
    }

    // Always remove from local list and update UI regardless of D-Bus result
    m_pairedDevices.removeIf([&address](const BTDevice &d) {
        return d.address == address;
    });

    if (m_connectedAddress == address) {
        m_connected = false;
        m_connectedName.clear();
        m_connectedAddress.clear();
        emit connectedChanged();
    }

    emit pairedDevicesChanged();

    QSettings settings("headunit", "bluetooth");
    if (settings.value("lastDevice").toString() == address)
        settings.remove("lastDevice");
}

void BluetoothManager::connectLastDevice()
{
    QSettings settings("headunit", "bluetooth");
    QString last = settings.value("lastDevice").toString();
    if (!last.isEmpty()) {
        qDebug() << "[BT] Auto-connecting to last device:" << last;
        connectDevice(last);
    }
}

void BluetoothManager::refreshPairedDevices()
{
    m_pairedDevices.clear();
    m_connected = false;
    m_connectedName.clear();
    m_connectedAddress.clear();

    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface objectManager("org.bluez", "/",
                                 "org.freedesktop.DBus.ObjectManager", bus);

    // Use typed reply so Qt handles the demarshalling — avoids read-only QDBusArgument crash
    QDBusReply<ManagedObjectsMap> reply = objectManager.call("GetManagedObjects");

    if (!reply.isValid()) {
        qWarning() << "[BT] GetManagedObjects failed:" << reply.error().message();
        return;
    }

    const ManagedObjectsMap &objects = reply.value();

    for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
        const InterfaceMap &interfaces = it.value();

        if (!interfaces.contains("org.bluez.Device1")) continue;

        const QVariantMap &props = interfaces["org.bluez.Device1"];
        if (!props.value("Paired").toBool()) continue;

        BTDevice dev;
        dev.address   = props.value("Address").toString();
        dev.name      = props.value("Name", "Unknown Device").toString();
        dev.connected = props.value("Connected").toBool();
        dev.paired    = true;

        if (dev.connected) {
            m_connected        = true;
            m_connectedName    = dev.name;
            m_connectedAddress = dev.address;
        }

        m_pairedDevices.append(dev);
        watchDeviceProperties(it.key().path());

        if (m_pairedDevices.size() >= MAX_DEVICES) break;
    }

    emit pairedDevicesChanged();
    emit connectedChanged();
}

QVariantList BluetoothManager::pairedDevicesVariant() const
{
    QVariantList list;
    for (const BTDevice &d : m_pairedDevices) {
        QVariantMap map;
        map["address"]   = d.address;
        map["name"]      = d.name.isEmpty() ? d.address : d.name;
        map["connected"] = d.connected;
        map["paired"]    = d.paired;
        list.append(map);
    }
    return list;
}

void BluetoothManager::onInterfacesAdded(
    const QDBusObjectPath &path,
    const QMap<QString, QVariantMap> &interfaces)
{
    if (!interfaces.contains("org.bluez.Device1")) return;

    const QVariantMap &props = interfaces["org.bluez.Device1"];
    QString address = props.value("Address").toString();
    QString name    = props.value("Name", "Unknown").toString();
    bool    paired  = props.value("Paired").toBool();

    emit deviceFound(address, name);

    if (paired) {
        for (const BTDevice &d : m_pairedDevices)
            if (d.address == address) return;

        if (m_pairedDevices.size() < MAX_DEVICES) {
            BTDevice dev;
            dev.address   = address;
            dev.name      = name;
            dev.connected = props.value("Connected").toBool();
            dev.paired    = true;
            m_pairedDevices.append(dev);
            watchDeviceProperties(path.path());
            emit pairedDevicesChanged();
        }
    }
}

void BluetoothManager::onInterfacesRemoved(
    const QDBusObjectPath &path,
    const QStringList &interfaces)
{
    Q_UNUSED(path)
    if (interfaces.contains("org.bluez.Device1"))
        refreshPairedDevices();
}

void BluetoothManager::onAdapterPropertiesChanged(
    const QString &iface,
    const QVariantMap &changed,
    const QStringList &)
{
    if (iface != "org.bluez.Adapter1") return;

    if (changed.contains("Powered"))
        emit poweredChanged(changed["Powered"].toBool());

    if (changed.contains("Discovering")) {
        m_discovering = changed["Discovering"].toBool();
        emit discoveringChanged(m_discovering);
    }

    if (changed.contains("Discoverable")) {
        m_discoverable = changed["Discoverable"].toBool();
        qDebug() << "[BT] Discoverable state:" << m_discoverable;
        emit discoverableChanged();
    }
}

void BluetoothManager::onDevicePropertiesChanged(
    const QString &iface,
    const QVariantMap &changed,
    const QStringList &)
{
    if (iface != "org.bluez.Device1") return;
    if (!changed.contains("Connected")) return;

    // Re-query all devices to get accurate state
    QDBusConnection bus = QDBusConnection::systemBus();
    bool anyConnected = false;

    for (BTDevice &dev : m_pairedDevices) {
        QDBusInterface props("org.bluez", devicePath(dev.address),
                             "org.freedesktop.DBus.Properties", bus);
        QDBusReply<QVariant> reply = props.call("Get", "org.bluez.Device1", "Connected");

        if (!reply.isValid()) continue;

        dev.connected = reply.value().toBool();

        if (dev.connected) {
            anyConnected       = true;
            m_connected        = true;
            m_connectedName    = dev.name;
            m_connectedAddress = dev.address;
        }
    }

    if (!anyConnected) {
        m_connected = false;
        m_connectedName.clear();
        m_connectedAddress.clear();
    }

    emit pairedDevicesChanged();
    if (!m_switching)
        emit connectedChanged();
}

QString BluetoothManager::devicePath(const QString &address) const
{
    return "/org/bluez/hci0/dev_" + QString(address).replace(":", "_");
}

// ── Discoverable ─────────────────────────────────────────────────────────────

void BluetoothManager::makeDiscoverable(int seconds)
{
    if (!m_adapter) return;

    // Set adapter Discoverable + DiscoverableTimeout via Properties.Set
    QDBusInterface props("org.bluez", m_adapter->path(),
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());
    props.call("Set", "org.bluez.Adapter1", "DiscoverableTimeout",
               QVariant::fromValue(QDBusVariant((quint32)seconds)));
    props.call("Set", "org.bluez.Adapter1", "Discoverable",
               QVariant::fromValue(QDBusVariant(true)));

    m_discoverable        = true;
    m_discoverableSeconds = seconds;
    emit discoverableChanged();
    qDebug() << "[BT] Discoverable for" << seconds << "seconds";

    // Countdown timer — fires every second to update the UI counter
    if (!m_discoverableTimer) {
        m_discoverableTimer = new QTimer(this);
        m_discoverableTimer->setInterval(1000);
        connect(m_discoverableTimer, &QTimer::timeout, this, [this]() {
            m_discoverableSeconds--;
            if (m_discoverableSeconds <= 0) {
                m_discoverableSeconds = 0;
                m_discoverable        = false;
                m_discoverableTimer->stop();
                qDebug() << "[BT] Discoverable window expired";
            }
            emit discoverableChanged();
        });
    }
    m_discoverableTimer->start();
}

// ── Network / carrier info ────────────────────────────────────────────────

void BluetoothManager::setModemPathForNetwork(const QString &modemPath)
{
    if (m_netModemPath == modemPath) return;
    m_netModemPath = modemPath;

    delete m_netreg;
    m_netreg = nullptr;

    if (m_netPollTimer) {
        m_netPollTimer->stop();
    } else {
        m_netPollTimer = new QTimer(this);
        m_netPollTimer->setInterval(15000);
        connect(m_netPollTimer, &QTimer::timeout, this, &BluetoothManager::refreshNetwork);
    }

    if (modemPath.isEmpty()) {
        m_netOperator.clear();
        m_netStrength    = 0;
        m_netTechnology.clear();
        m_netRoaming     = false;
        emit networkChanged();
        return;
    }

    m_netreg = new QDBusInterface("org.ofono", modemPath,
                                  "org.ofono.NetworkRegistration",
                                  QDBusConnection::systemBus(), this);

    if (!m_netreg->isValid()) {
        qWarning() << "[Network] NetworkRegistration not available at" << modemPath;
        delete m_netreg;
        m_netreg = nullptr;
        return;
    }

    // oFono signal: PropertyChanged(string name, variant value)
    QDBusConnection::systemBus().connect(
        "org.ofono", modemPath,
        "org.ofono.NetworkRegistration", "PropertyChanged",
        this, SLOT(refreshNetwork()));

    refreshNetwork();
    m_netPollTimer->start();
    qDebug() << "[Network] Monitoring network at" << modemPath;
}

void BluetoothManager::refreshNetwork()
{
    if (!m_netreg) return;

    QDBusMessage req = QDBusMessage::createMethodCall(
        "org.ofono", m_netModemPath,
        "org.ofono.NetworkRegistration", "GetProperties");
    QDBusMessage reply = QDBusConnection::systemBus().call(req);

    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
        return;

    // Copy arg out of the reply before iterating — reply-owned args are read-only
    QDBusArgument rawArg = reply.arguments().first().value<QDBusArgument>();
    QMap<QString, QDBusVariant> rawMap;
    rawArg >> rawMap;

    QVariantMap props;
    for (auto it = rawMap.cbegin(); it != rawMap.cend(); ++it)
        props.insert(it.key(), it.value().variant());

    // HFP modem only exposes: Name, Strength, Status, Mode
    // Technology, Roaming, Battery not available over HFP
    QString newOp  = props.value("Name",     "").toString();
    // Strength is a byte 0-100, convert to 0-5 bars
    int     newSig = qRound(props.value("Strength", 0).toDouble() / 20.0);

    bool changed = (newOp != m_netOperator || newSig != m_netStrength);

    if (changed) {
        m_netOperator = newOp;
        m_netStrength = newSig;
        qDebug() << "[Network]" << m_netOperator
                 << "signal:" << m_netStrength << "/5";
        emit networkChanged();
    }
}
