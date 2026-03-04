#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include "uartController.h"
#include "carplayController.h"
#include "canController.h"
#include "BluetoothManager.h"
#include "CallManager.h"
#include "ContactsManager.h"
#include "BluetoothMediaPlayer.h"
#include "SystemClock.h"
#include "ambientController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("headunit");
    app.setOrganizationName("headunit");

    QQmlApplicationEngine engine;

    // ── Existing controllers ──────────────────────────────────────────────────
    UARTController uartController;
    engine.rootContext()->setContextProperty("uartController", &uartController);
    uartController.openPort("/dev/ttyAMA10");

    CarPlayController carplayController;
    engine.rootContext()->setContextProperty("carplayController", &carplayController);

    CANController canController;
    engine.rootContext()->setContextProperty("canController", &canController);

    // ── Bluetooth / Phone controllers ─────────────────────────────────────────
    BluetoothManager     bluetoothManager;
    CallManager          callManager;
    ContactsManager      contactsManager;
    BluetoothMediaPlayer mediaPlayer;
    SystemClock          systemClock;
    AmbientController    ambientController;

    engine.rootContext()->setContextProperty("bluetoothManager",  &bluetoothManager);
    engine.rootContext()->setContextProperty("callManager",       &callManager);
    engine.rootContext()->setContextProperty("contactsManager",   &contactsManager);
    engine.rootContext()->setContextProperty("mediaPlayer",        &mediaPlayer);
    engine.rootContext()->setContextProperty("systemClock",         &systemClock);
    engine.rootContext()->setContextProperty("ambientController",   &ambientController);

    // ── Wire: phone connects → set oFono modem path + auto-sync contacts ──────
    // Use raw pointers — lambdas can't safely capture local references across
    // a nested QTimer::singleShot boundary
    CallManager      *callMgrPtr     = &callManager;
    ContactsManager  *contactsMgrPtr = &contactsManager;

    // Tracks last connected address to prevent duplicate sync on startup
    // (refreshPairedDevices + connectLastDevice both fire connectedChanged)
    static QString lastSyncedAddr;

    QObject::connect(
        &bluetoothManager, &BluetoothManager::connectedChanged,
        [&bluetoothManager, callMgrPtr, contactsMgrPtr, &mediaPlayer]() {
            if (!bluetoothManager.isConnected()) {
                // Only clear if we had actually synced
                if (!lastSyncedAddr.isEmpty()) {
                    contactsMgrPtr->clearContacts();
                }
                // Always clear so next connection triggers fresh sync
                lastSyncedAddr.clear();
                return;
            }

            QString connectedAddr;
            for (const QVariant &v : bluetoothManager.pairedDevicesVariant()) {
                QVariantMap m = v.toMap();
                if (m["connected"].toBool()) {
                    connectedAddr = m["address"].toString();
                    break;
                }
            }

            if (connectedAddr.isEmpty()) return;

            QString modemPath = "/hfp/org/bluez/hci0/dev_" +
                                QString(connectedAddr).replace(":", "_");

            qDebug() << "[Main] Phone connected. Modem path:" << modemPath;
            callMgrPtr->setModemPath(modemPath);
            bluetoothManager.setModemPathForNetwork(modemPath);

            // Set AVRCP media player path
            // Pass base device path — BluetoothMediaPlayer appends /player0 etc.
            QString mediaBasePath = "/org/bluez/hci0/dev_" +
                                    QString(connectedAddr).replace(":", "_");
            mediaPlayer.setPlayerPath(mediaBasePath);

            // Deduplicate: skip if already syncing/synced this address
            if (lastSyncedAddr == connectedAddr) {
                qDebug() << "[Main] Already synced" << connectedAddr << "— skipping";
                return;
            }
            lastSyncedAddr = connectedAddr;

            // 5s delay — iOS needs time after HFP to make PBAP available
            QTimer::singleShot(5000, [contactsMgrPtr, connectedAddr]() {
                contactsMgrPtr->syncContacts(connectedAddr);
            });
        }
        );

    // ── CarPlay ↔ Bluetooth coordination ─────────────────────────────────────
    // Disconnect BT when CarPlay starts (avoids audio/HFP conflicts)
    QObject::connect(&carplayController, &CarPlayController::carplayStarted,
                     [&bluetoothManager]() {
                         qDebug() << "[Main] CarPlay started — disconnecting Bluetooth";
                         if (!bluetoothManager.connectedAddress().isEmpty())
                             bluetoothManager.disconnectDevice(bluetoothManager.connectedAddress());
                     });

    // Reconnect last BT device when CarPlay stops
    BluetoothManager *btPtr = &bluetoothManager;
    QObject::connect(&carplayController, &CarPlayController::carplayStopped,
                     [btPtr]() {
                         qDebug() << "[Main] CarPlay stopped — reconnecting Bluetooth";
                         QTimer::singleShot(1500, btPtr, &BluetoothManager::connectLastDevice);
                     });

    // Return to home screen when CarPlay exits with code 42
    QObject::connect(&carplayController, &CarPlayController::shouldReturnHome, [&engine]() {
        QObject *rootObject = engine.rootObjects().first();
        if (rootObject) {
            QObject *stackView = rootObject->findChild<QObject*>("stackView");
            if (stackView) {
                QMetaObject::invokeMethod(stackView, "pop", Q_ARG(QVariant, QVariant()));
            }
        }
    });

    // ── Load QML ──────────────────────────────────────────────────────────────
    const QUrl url(QStringLiteral("qrc:/qt/qml/Application/Main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    // ── Auto-connect to last known device 2s after startup ───────────────────
    QTimer::singleShot(2000, &bluetoothManager, &BluetoothManager::connectLastDevice);

    return app.exec();
}
