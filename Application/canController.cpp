#include "canController.h"
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QMutexLocker>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <linux/can.h>
#include <linux/can/raw.h>

// ===== CANController Implementation =====

CANController::CANController(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
    , m_messageCount(0)
    , m_busLoad(0)
    , m_isPaused(false)
    , m_readerThread(nullptr)
    , m_lastMessageCount(0)
    , m_hasPendingUpdates(false)
{
    m_busLoadTimer = new QTimer(this);
    connect(m_busLoadTimer, &QTimer::timeout, this, &CANController::updateBusLoad);
    m_busLoadTimer->start(1000);  // Update bus load every second

    // Throttle UI updates to prevent freezing
    m_uiUpdateTimer = new QTimer(this);
    connect(m_uiUpdateTimer, &QTimer::timeout, this, &CANController::updateUIMessages);
    m_uiUpdateTimer->start(100);  // Update UI every 100ms instead of every message
}

CANController::~CANController()
{
    disconnectCAN();
}

bool CANController::connectToCAN(const QString &interface)
{
    if (m_isConnected) {
        qDebug() << "Already connected to CAN";
        return true;
    }

    // Create and start reader thread
    m_readerThread = new CANReaderThread(interface, this);

    connect(m_readerThread, &CANReaderThread::messageReceived,
            this, &CANController::onNewCANMessage);

    connect(m_readerThread, &CANReaderThread::errorOccurred,
            [this](const QString &error) {
                qDebug() << "CAN Error:" << error;
                disconnectCAN();
            });

    connect(m_readerThread, &CANReaderThread::finished,
            m_readerThread, &QObject::deleteLater);

    m_readerThread->start();

    m_isConnected = true;
    emit connectionChanged();

    qDebug() << "Connected to CAN interface:" << interface;
    return true;
}

void CANController::disconnectCAN()
{
    if (m_readerThread) {
        m_readerThread->stop();
        m_readerThread->wait(1000);  // Wait up to 1 second
        if (m_readerThread->isRunning()) {
            m_readerThread->terminate();
            m_readerThread->wait();
        }
        m_readerThread = nullptr;
    }

    m_isConnected = false;
    emit connectionChanged();
}

void CANController::onNewCANMessage(QVariantMap message)
{
    if (m_isPaused) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    // Add to pending messages buffer instead of directly to display list
    m_pendingMessages.append(message);
    m_messageCount++;
    m_hasPendingUpdates = true;

    // Don't emit rawMessagesChanged here - let the timer do it
    emit messageCountChanged();
}

void CANController::updateUIMessages()
{
    if (!m_hasPendingUpdates) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    // Move pending messages to display list
    m_rawMessages.append(m_pendingMessages);
    m_pendingMessages.clear();

    // Limit list size to prevent memory issues (keep last 1000 messages)
    while (m_rawMessages.count() > 200) {
        m_rawMessages.removeFirst();
    }

    m_hasPendingUpdates = false;

    // Emit signal to update UI (only once per timer tick)
    emit rawMessagesChanged();
    emit rawMessageCountChanged();
}

void CANController::updateBusLoad()
{
    int currentCount = m_messageCount;
    int messagesPerSecond = currentCount - m_lastMessageCount;
    m_lastMessageCount = currentCount;

    // Estimate bus load (assuming 500kbit/s CAN and average 10 bytes per message)
    // Max theoretical: ~6000 messages/second
    m_busLoad = qMin(100, (messagesPerSecond * 100) / 6000);
    emit busLoadChanged();
}

void CANController::togglePause()
{
    m_isPaused = !m_isPaused;
    emit pausedChanged();
    qDebug() << "CAN monitoring" << (m_isPaused ? "paused" : "resumed");
}

void CANController::clearMessages()
{
    QMutexLocker locker(&m_mutex);
    m_rawMessages.clear();
    m_pendingMessages.clear();
    m_messageCount = 0;
    m_hasPendingUpdates = false;
    emit rawMessagesChanged();
    emit rawMessageCountChanged();
    emit messageCountChanged();
    qDebug() << "CAN messages cleared";
}

void CANController::exportToCSV(const QString &filename)
{
    QMutexLocker locker(&m_mutex);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for export:" << filename;
        return;
    }

    QTextStream out(&file);

    // Write CSV header
    out << "Timestamp,CAN ID,DLC,Data\n";

    // Write each message
    for (const QVariant &var : m_rawMessages) {
        QVariantMap msg = var.toMap();
        out << msg["timestamp"].toString() << ","
            << msg["canId"].toString() << ","
            << msg["dlc"].toString() << ","
            << msg["data"].toString() << "\n";
    }

    file.close();
    qDebug() << "Exported" << m_rawMessages.count() << "messages to" << filename;
}

QVariantList CANController::rawMessages() const
{
    QMutexLocker locker(&m_mutex);
    return m_rawMessages;
}

int CANController::rawMessageCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_rawMessages.count();
}

// ===== CANReaderThread Implementation =====

CANReaderThread::CANReaderThread(const QString &interface, QObject *parent)
    : QThread(parent)
    , m_interface(interface)
    , m_running(false)
    , m_canSocket(-1)
{
}

CANReaderThread::~CANReaderThread()
{
    stop();
}

void CANReaderThread::stop()
{
    m_running = false;
}

void CANReaderThread::run()
{
    // Create socket
    m_canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_canSocket < 0) {
        emit errorOccurred("Failed to create CAN socket");
        return;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(m_canSocket, F_GETFL, 0);
    fcntl(m_canSocket, F_SETFL, flags | O_NONBLOCK);

    // Get interface index
    struct ifreq ifr;
    strncpy(ifr.ifr_name, m_interface.toStdString().c_str(), IFNAMSIZ - 1);
    if (ioctl(m_canSocket, SIOCGIFINDEX, &ifr) < 0) {
        emit errorOccurred(QString("Failed to get interface index for %1").arg(m_interface));
        close(m_canSocket);
        return;
    }

    // Bind socket
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(m_canSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        emit errorOccurred("Failed to bind CAN socket");
        close(m_canSocket);
        return;
    }

    qDebug() << "CAN reader thread started for" << m_interface;

    m_running = true;
    struct can_frame frame;

    while (m_running) {
        ssize_t nbytes = read(m_canSocket, &frame, sizeof(struct can_frame));

        if (nbytes > 0) {
            if (nbytes == sizeof(struct can_frame)) {
                // Create message map
                QVariantMap message;
                message["timestamp"] = getCurrentTimestamp();
                message["canId"] = QString("0x%1").arg(frame.can_id & CAN_EFF_MASK, 3, 16, QChar('0')).toUpper();
                message["dlc"] = frame.can_dlc;
                message["data"] = formatCANData(frame.data, frame.can_dlc);

                emit messageReceived(message);
            }
        } else if (nbytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            // Real error occurred
            qDebug() << "CAN read error:" << strerror(errno);
        }

        // Small sleep to prevent CPU spinning
        msleep(1);
    }

    close(m_canSocket);
    qDebug() << "CAN reader thread stopped";
}

QString CANReaderThread::formatCANData(const unsigned char *data, int dlc)
{
    QString result;
    for (int i = 0; i < dlc; i++) {
        result += QString("%1 ").arg(data[i], 2, 16, QChar('0')).toUpper();
    }
    return result.trimmed();
}

QString CANReaderThread::getCurrentTimestamp()
{
    QDateTime now = QDateTime::currentDateTime();
    return now.toString("hh:mm:ss.zzz");
}
