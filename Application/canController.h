#ifndef CANCONTROLLER_H
#define CANCONTROLLER_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>
#include <QThread>
#include <QFile>
#include <QMutex>

// Forward declaration
class CANReaderThread;

class CANController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(int messageCount READ messageCount NOTIFY messageCountChanged)
    Q_PROPERTY(int rawMessageCount READ rawMessageCount NOTIFY rawMessageCountChanged)
    Q_PROPERTY(int busLoad READ busLoad NOTIFY busLoadChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY pausedChanged)
    Q_PROPERTY(QVariantList rawMessages READ rawMessages NOTIFY rawMessagesChanged)

public:
    explicit CANController(QObject *parent = nullptr);
    ~CANController();

    Q_INVOKABLE bool connectToCAN(const QString &interface = "can0");
    Q_INVOKABLE void disconnectCAN();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void clearMessages();
    Q_INVOKABLE void exportToCSV(const QString &filename);

    bool isConnected() const { return m_isConnected; }
    int messageCount() const { return m_messageCount; }
    int rawMessageCount() const;
    int busLoad() const { return m_busLoad; }
    bool isPaused() const { return m_isPaused; }
    QVariantList rawMessages() const;

signals:
    void connectionChanged();
    void messageCountChanged();
    void rawMessageCountChanged();
    void busLoadChanged();
    void pausedChanged();
    void rawMessagesChanged();
    void newMessage(QVariantMap message);

public slots:
    void onNewCANMessage(QVariantMap message);

private slots:
    void updateBusLoad();
    void updateUIMessages();  // New: throttled UI update

private:
    bool m_isConnected;
    int m_messageCount;
    int m_busLoad;
    bool m_isPaused;
    QVariantList m_rawMessages;
    QVariantList m_pendingMessages;  // New: buffer for messages
    QTimer *m_busLoadTimer;
    QTimer *m_uiUpdateTimer;  // New: throttle UI updates
    CANReaderThread *m_readerThread;
    mutable QMutex m_mutex;
    int m_lastMessageCount;
    bool m_hasPendingUpdates;  // New: flag for pending updates
};

// Separate thread for reading CAN messages
class CANReaderThread : public QThread
{
    Q_OBJECT

public:
    explicit CANReaderThread(const QString &interface, QObject *parent = nullptr);
    ~CANReaderThread();

    void stop();

signals:
    void messageReceived(QVariantMap message);
    void errorOccurred(QString error);

protected:
    void run() override;

private:
    QString m_interface;
    bool m_running;
    int m_canSocket;

    QString formatCANData(const unsigned char *data, int dlc);
    QString getCurrentTimestamp();
};

#endif
