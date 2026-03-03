#include "BluetoothManager.h"

#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QSettings>
#include <QDebug>

// Type aliases and Q_DECLARE_METATYPE are in BluetoothManager.h

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<InterfaceMap>();
    qDBusRegisterMetaType<ManagedObjectsMap>();

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
    if (!m_adapter->isValid()) return;
    m_adapter->call("StartDiscovery");
}

void BluetoothManager::stopDiscovery()
{
    if (!m_adapter->isValid()) return;
    m_adapter->call("StopDiscovery");
}

void BluetoothManager::connectDevice(const QString &address)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface device("org.bluez", devicePath(address), "org.bluez.Device1", bus);

    if (!device.isValid()) {
        emit errorOccurred("Device not found: " + address);
        return;
    }

    device.asyncCall("Connect");

    QSettings settings("headunit", "bluetooth");
    settings.setValue("lastDevice", address);
}

void BluetoothManager::disconnectDevice(const QString &address)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface device("org.bluez", devicePath(address), "org.bluez.Device1", bus);
    device.asyncCall("Disconnect");
}

void BluetoothManager::removeDevice(const QString &address)
{
    m_adapter->asyncCall("RemoveDevice",
                         QVariant::fromValue(QDBusObjectPath(devicePath(address))));

    m_pairedDevices.removeIf([&address](const BTDevice &d) {
        return d.address == address;
    });
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
    emit connectedChanged();
}

QString BluetoothManager::devicePath(const QString &address) const
{
    return "/org/bluez/hci0/dev_" + QString(address).replace(":", "_");
}
