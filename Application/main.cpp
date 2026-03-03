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
    BluetoothManager bluetoothManager;
    CallManager      callManager;
    ContactsManager  contactsManager;

    engine.rootContext()->setContextProperty("bluetoothManager",  &bluetoothManager);
    engine.rootContext()->setContextProperty("callManager",       &callManager);
    engine.rootContext()->setContextProperty("contactsManager",   &contactsManager);

    // ── Wire: phone connects → set oFono modem path + auto-sync contacts ──────
    // Use raw pointers — lambdas can't safely capture local references across
    // a nested QTimer::singleShot boundary
    CallManager      *callMgrPtr     = &callManager;
    ContactsManager  *contactsMgrPtr = &contactsManager;

    QObject::connect(
        &bluetoothManager, &BluetoothManager::connectedChanged,
        [&bluetoothManager, callMgrPtr, contactsMgrPtr]() {
            if (!bluetoothManager.isConnected()) return;

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

            // Delay sync 3s to let HFP service level connection settle first
            QTimer::singleShot(3000, [contactsMgrPtr, connectedAddr]() {
                contactsMgrPtr->syncContacts(connectedAddr);
            });
        }
        );

    // ── Existing CarPlay signal ───────────────────────────────────────────────
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

    // Check if a device is already connected at startup
    QTimer::singleShot(3000, [&bluetoothManager, callMgrPtr, contactsMgrPtr]() {
        if (!bluetoothManager.isConnected()) return;

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

        qDebug() << "[Main] Already connected at startup. Modem path:" << modemPath;
        callMgrPtr->setModemPath(modemPath);

        QTimer::singleShot(3000, [contactsMgrPtr, connectedAddr]() {
            contactsMgrPtr->syncContacts(connectedAddr);
        });
    });

    return app.exec();
}
