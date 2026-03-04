#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QVariantMap>
#include <QStringList>
#include <QTimer>

// ── Type aliases for GetManagedObjects typed reply ────────────────────────
typedef QMap<QString, QVariantMap>          InterfaceMap;
typedef QMap<QDBusObjectPath, InterfaceMap> ManagedObjectsMap;

Q_DECLARE_METATYPE(InterfaceMap)
Q_DECLARE_METATYPE(ManagedObjectsMap)

struct BTDevice {
    QString address;
    QString name;
    bool    connected = false;
    bool    paired    = false;
};

class BluetoothManager : public QObject
{
    Q_OBJECT

    // ── Bluetooth ─────────────────────────────────────────────────────────
    Q_PROPERTY(bool         powered            READ isPowered           NOTIFY poweredChanged)
    Q_PROPERTY(bool         discovering        READ isDiscovering       NOTIFY discoveringChanged)
    Q_PROPERTY(bool         discoverable       READ isDiscoverable      NOTIFY discoverableChanged)
    Q_PROPERTY(int          discoverableSeconds READ discoverableSeconds NOTIFY discoverableChanged)
    Q_PROPERTY(bool         connected          READ isConnected         NOTIFY connectedChanged)
    Q_PROPERTY(QString      connectedName      READ connectedName       NOTIFY connectedChanged)
    Q_PROPERTY(QVariantList pairedDevices      READ pairedDevicesVariant NOTIFY pairedDevicesChanged)

    // ── Network ───────────────────────────────────────────────────────────
    Q_PROPERTY(QString networkOperator   READ networkOperator   NOTIFY networkChanged)
    Q_PROPERTY(int     networkStrength   READ networkStrength   NOTIFY networkChanged)
    Q_PROPERTY(QString networkTechnology READ networkTechnology NOTIFY networkChanged)
    Q_PROPERTY(bool    networkRoaming    READ networkRoaming    NOTIFY networkChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    // Bluetooth
    bool         isPowered()           const;
    bool         isDiscovering()       const { return m_discovering; }
    bool         isDiscoverable()      const { return m_discoverable; }
    int          discoverableSeconds() const { return m_discoverableSeconds; }
    bool         isConnected()         const { return m_connected; }
    QString      connectedName()       const { return m_connectedName; }
    QString      connectedAddress()    const { return m_connectedAddress; }
    QVariantList pairedDevicesVariant() const;

    // Network
    QString networkOperator()   const { return m_netOperator; }
    int     networkStrength()   const { return m_netStrength; }
    QString networkTechnology() const { return m_netTechnology; }
    bool    networkRoaming()    const { return m_netRoaming; }

public slots:
    void setPower(bool on);
    void startDiscovery();
    void stopDiscovery();
    void connectDevice(const QString &address);
    void disconnectDevice(const QString &address);
    void removeDevice(const QString &address);
    void connectLastDevice();
    void refreshPairedDevices();
    void makeDiscoverable(int seconds = 30);
    void setModemPathForNetwork(const QString &modemPath);

signals:
    void poweredChanged(bool on);
    void discoveringChanged(bool discovering);
    void discoverableChanged();
    void connectedChanged();
    void pairedDevicesChanged();
    void deviceFound(const QString &address, const QString &name);
    void errorOccurred(const QString &message);
    void networkChanged();

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
    void refreshNetwork();

private:
    void    setupSignals();
    void    doConnect(const QString &address);
    void    watchDeviceProperties(const QString &dbusPath);
    QString devicePath(const QString &address) const;

    // Bluetooth state
    QDBusInterface  *m_adapter             = nullptr;
    bool             m_discovering         = false;
    bool             m_connected           = false;
    bool             m_discoverable        = false;
    bool             m_switching           = false;  // true while swapping devices
    int              m_discoverableSeconds = 0;
    QTimer          *m_discoverableTimer   = nullptr;
    QString          m_connectedName;
    QString          m_connectedAddress;
    QList<BTDevice>  m_pairedDevices;

    // Network state
    QDBusInterface  *m_netreg          = nullptr;
    QTimer          *m_netPollTimer    = nullptr;
    QString          m_netModemPath;
    QString          m_netOperator;
    int              m_netStrength     = 0;
    QString          m_netTechnology;
    bool             m_netRoaming      = false;

    static constexpr int MAX_DEVICES = 5;
};

#endif // BLUETOOTHMANAGER_H
