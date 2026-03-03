#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QVariantMap>
#include <QStringList>

// ── Type aliases for GetManagedObjects typed reply ─────────────────────────
typedef QMap<QString, QVariantMap>              InterfaceMap;
typedef QMap<QDBusObjectPath, InterfaceMap>     ManagedObjectsMap;

Q_DECLARE_METATYPE(InterfaceMap)
Q_DECLARE_METATYPE(ManagedObjectsMap)

// ── Simple device info struct exposed to QML ──────────────────────────────
struct BTDevice {
    QString address;
    QString name;
    bool    connected = false;
    bool    paired    = false;
};

// ── Manager ────────────────────────────────────────────────────────────────
class BluetoothManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool    powered       READ isPowered     NOTIFY poweredChanged)
    Q_PROPERTY(bool    discovering   READ isDiscovering NOTIFY discoveringChanged)
    Q_PROPERTY(bool    connected     READ isConnected   NOTIFY connectedChanged)
    Q_PROPERTY(QString connectedName READ connectedName NOTIFY connectedChanged)
    Q_PROPERTY(QVariantList pairedDevices READ pairedDevicesVariant NOTIFY pairedDevicesChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    bool    isPowered()     const;
    bool    isDiscovering() const { return m_discovering; }
    bool    isConnected()   const { return m_connected; }
    QString connectedName() const { return m_connectedName; }

    QVariantList pairedDevicesVariant() const;

public slots:
    void setPower(bool on);
    void startDiscovery();
    void stopDiscovery();
    void connectDevice(const QString &address);
    void disconnectDevice(const QString &address);
    void removeDevice(const QString &address);
    void connectLastDevice();
    void refreshPairedDevices();

signals:
    void poweredChanged(bool on);
    void discoveringChanged(bool discovering);
    void connectedChanged();
    void pairedDevicesChanged();
    void deviceFound(const QString &address, const QString &name);
    void errorOccurred(const QString &message);

private slots:
    void onInterfacesAdded(const QDBusObjectPath &path,
                           const QMap<QString, QVariantMap> &interfaces);
    void onInterfacesRemoved(const QDBusObjectPath &path,
                             const QStringList &interfaces);
    void onAdapterPropertiesChanged(const QString &iface,
                                    const QVariantMap &changed,
                                    const QStringList &invalidated);
    void onDevicePropertiesChanged(const QString &iface,
                                   const QVariantMap &changed,
                                   const QStringList &invalidated);

private:
    void setupSignals();
    void watchDeviceProperties(const QString &dbusPath);
    QString devicePath(const QString &address) const;

    QDBusInterface  *m_adapter         = nullptr;
    bool             m_discovering     = false;
    bool             m_connected       = false;
    QString          m_connectedName;
    QString          m_connectedAddress;
    QList<BTDevice>  m_pairedDevices;

    static constexpr int MAX_DEVICES = 5;
};

#endif // BLUETOOTHMANAGER_H
