#include "uartController.h"
#include <QDebug>
#include <cmath>

static const int ACK_TIMEOUT_MS = 200;

UARTController::UARTController(QObject *parent) : QObject(parent)
{
    serialPort = new QSerialPort(this);
    connect(serialPort, &QSerialPort::readyRead,     this, &UARTController::handleReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &UARTController::handleError);

    ackTimer = new QTimer(this);
    ackTimer->setSingleShot(true);
    connect(ackTimer, &QTimer::timeout, this, &UARTController::onAckTimeout);
}

UARTController::~UARTController()
{
    if (serialPort->isOpen())
        serialPort->close();
}

bool UARTController::openPort(const QString &portName)
{
    serialPort->setPortName(portName);
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "[UART] Port opened:" << portName;
        emit connectionStatusChanged(true);
        return true;
    }
    qDebug() << "[UART] Failed to open port:" << serialPort->errorString();
    emit connectionStatusChanged(false);
    return false;
}

void UARTController::closePort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        emit connectionStatusChanged(false);
    }
}

// ── CRC8 (poly 0x07) ─────────────────────────────────────────────────────────
quint8 UARTController::calculateCRC8(const QByteArray &data)
{
    quint8 crc = 0x00;
    for (quint8 byte : data) {
        crc ^= byte;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1;
    }
    return crc;
}

// ── Packet builder ────────────────────────────────────────────────────────────
QByteArray UARTController::buildPacket(quint8 cmd, const QByteArray &data)
{
    quint8 actualLen = static_cast<quint8>(qMin(data.size(), 16));

    QByteArray paddedData = data.left(16);
    while (paddedData.size() < 16)
        paddedData.append('\x00');

    QByteArray crcInput;
    crcInput.append(static_cast<char>(cmd));
    crcInput.append(static_cast<char>(actualLen));
    crcInput.append(paddedData);

    QByteArray packet;
    packet.append('\xAA');
    packet.append(static_cast<char>(cmd));
    packet.append(static_cast<char>(actualLen));
    packet.append(paddedData);
    packet.append(static_cast<char>(calculateCRC8(crcInput)));
    return packet;
}

// ── Debug helper ──────────────────────────────────────────────────────────────
void UARTController::debugPacket(const QByteArray &packet)
{
    quint8 cmd = static_cast<quint8>(packet[1]);
    quint8 len = static_cast<quint8>(packet[2]);
    QString dbg = QString("[UART] TX CMD=0x%1 LEN=%2 | ")
                      .arg(cmd, 2, 16, QChar('0')).toUpper().arg(len);
    for (quint8 b : packet)
        dbg += QString("0x%1 ").arg(b, 2, 16, QChar('0')).toUpper();
    qDebug() << dbg;
}

// ── Queue management ──────────────────────────────────────────────────────────
void UARTController::enqueue(quint8 cmd, const QByteArray &data)
{
    // Replace any pending packet with same CMD rather than stacking duplicates
    // This keeps volume/temp/fan always up to date without queue buildup
    for (auto &pkt : sendQueue) {
        if (pkt.cmd == cmd) {
            pkt.data = data;
            qDebug() << "[UART] Queue: replaced pending CMD" << Qt::hex << cmd;
            return;
        }
    }

    PendingPacket pkt;
    pkt.cmd  = cmd;
    pkt.data = data;
    sendQueue.enqueue(pkt);
    qDebug() << "[UART] Queue: enqueued CMD" << Qt::hex << cmd
             << "— depth:" << sendQueue.size();

    if (!inFlight)
        sendNext();
}

void UARTController::sendNext()
{
    if (sendQueue.isEmpty() || inFlight)
        return;

    currentPacket = sendQueue.dequeue();
    QByteArray packet = buildPacket(currentPacket.cmd, currentPacket.data);
    serialPort->write(packet);
    debugPacket(packet);

    inFlight   = true;
    retryCount = 0;
    ackTimer->start(ACK_TIMEOUT_MS);
}

void UARTController::onAckTimeout()
{
    retryCount++;
    if (retryCount <= MAX_RETRIES) {
        qDebug() << "[UART] ACK timeout for CMD" << Qt::hex << currentPacket.cmd
                 << "— retry" << retryCount << "of" << MAX_RETRIES;
        QByteArray packet = buildPacket(currentPacket.cmd, currentPacket.data);
        serialPort->write(packet);
        debugPacket(packet);
        ackTimer->start(ACK_TIMEOUT_MS);
    } else {
        qDebug() << "[UART] CMD" << Qt::hex << currentPacket.cmd
                 << "giving up after" << MAX_RETRIES << "retries";
        inFlight = false;
        sendNext();
    }
}

void UARTController::handleAck(quint8 cmd)
{
    if (!inFlight) {
        qDebug() << "[UART] Unexpected ACK for CMD" << Qt::hex << cmd;
        return;
    }
    if (cmd != currentPacket.cmd) {
        qDebug() << "[UART] ACK CMD mismatch — expected"
                 << Qt::hex << currentPacket.cmd << "got" << cmd;
    }
    ackTimer->stop();
    inFlight = false;
    qDebug() << "[UART] ACK received for CMD" << Qt::hex << cmd
             << "— queue depth:" << sendQueue.size();
    sendNext();
}

// ── Public transmit ───────────────────────────────────────────────────────────
void UARTController::send(quint8 cmd, quint8 data)
{
    QByteArray d;
    d.append(static_cast<char>(data));
    enqueue(cmd, d);
}

void UARTController::sendData(quint8 cmd, const QByteArray &data)
{
    enqueue(cmd, data);
}

void UARTController::sendLCDText(const QString &text)
{
    enqueue(0x03, text.toLatin1());
}

// ── Special encodings ─────────────────────────────────────────────────────────
void UARTController::sendTemperature(quint8 cmd, float temperature)
{
    temperature = qBound(15.0f, temperature, 32.0f);
    temperature = roundf(temperature * 2.0f) / 2.0f;

    int t = static_cast<int>(roundf(temperature * 10.0f));
    QByteArray data;
    data.append(static_cast<char>((t / 100) % 10));
    data.append(static_cast<char>((t / 10)  % 10));
    data.append(static_cast<char>( t        % 10));
    enqueue(cmd, data);
}

void UARTController::sendAudioSettings(int bass, int treble, int mids, int fader, int balance)
{
    QByteArray data;
    data.append(static_cast<char>(bass    + 5));
    data.append(static_cast<char>(treble  + 5));
    data.append(static_cast<char>(mids    + 5));
    data.append(static_cast<char>(fader   + 5));
    data.append(static_cast<char>(balance + 5));
    enqueue(0x15, data);
}

void UARTController::sendAmbient(int r, int g, int b, int brightness, bool on)
{
    QByteArray d;
    d.append(static_cast<char>(r));
    d.append(static_cast<char>(g));
    d.append(static_cast<char>(b));
    d.append(static_cast<char>(brightness));
    d.append(static_cast<char>(on ? 1 : 0));
    enqueue(0x18, d);
}

void UARTController::sendTime(int hour, int min, int second, int day, int month, int year, int format)
{
    QByteArray d;
    d.append(static_cast<char>(hour));
    d.append(static_cast<char>(min));
    d.append(static_cast<char>(second));
    d.append(static_cast<char>(day));
    d.append(static_cast<char>(month));
    d.append(static_cast<char>(year));
    d.append(static_cast<char>(format)); //0 = 24, 1 = 12
    enqueue(0x1C, d);
}

// ── Volume slot (kept for signal compatibility) ───────────────────────────────
void UARTController::onVolumeChanged(int volume)
{
    send(0x0F, static_cast<quint8>(qBound(0, volume, 30)));
}

// ── RX ────────────────────────────────────────────────────────────────────────
void UARTController::handleReadyRead()
{
    rxBuffer.append(serialPort->readAll());

    QString dbg = "[UART] RX | ";
    for (quint8 b : rxBuffer)
        dbg += QString("0x%1 ").arg(b, 2, 16, QChar('0')).toUpper();
    qDebug() << dbg;

    while (rxBuffer.size() >= 20)
    {
        int startIdx = rxBuffer.indexOf('\xBB');
        if (startIdx < 0) { rxBuffer.clear(); break; }
        if (startIdx > 0)  rxBuffer.remove(0, startIdx);
        if (rxBuffer.size() < 20) break;

        quint8 cmd = static_cast<quint8>(rxBuffer[1]);
        quint8 len = static_cast<quint8>(rxBuffer[2]);

        QByteArray crcInput;
        crcInput.append(rxBuffer[1]);
        crcInput.append(rxBuffer[2]);
        crcInput.append(rxBuffer.mid(3, 16));
        quint8 expectedCRC = calculateCRC8(crcInput);
        quint8 receivedCRC = static_cast<quint8>(rxBuffer[19]);

        if (expectedCRC != receivedCRC) {
            qDebug() << "[UART] CRC mismatch — expected"
                     << Qt::hex << expectedCRC << "got" << receivedCRC;
            rxBuffer.remove(0, 1);
            continue;
        }

        QByteArray data = rxBuffer.mid(3, len);

        switch (cmd)
        {
        case 0x0E:  // Climate status — unsolicited, not an ACK
            if (data.size() >= 12)
            {
                ClimateStatus s;
                s.tempTens    = static_cast<quint8>(data[0]);
                s.tempUnits   = static_cast<quint8>(data[1]);
                s.tempDecimal = static_cast<quint8>(data[2]);
                s.fanSpeed    = static_cast<quint8>(data[3]);
                s.sysMode     = static_cast<quint8>(data[4]);
                s.ventMode    = static_cast<quint8>(data[5]);
                s.circMode    = static_cast<quint8>(data[6]);
                s.autoEnabled = static_cast<quint8>(data[7]);
                s.acEnabled   = static_cast<quint8>(data[8]);
                s.ecoEnabled  = static_cast<quint8>(data[9]);
                s.demistFront = static_cast<quint8>(data[10]);
                s.demistRear  = static_cast<quint8>(data[11]);
                qDebug() << "[UART] Climate status — Temp:"
                         << s.tempTens << s.tempUnits << "." << s.tempDecimal
                         << "Fan:" << s.fanSpeed
                         << "Vent:" << s.ventMode
                         << "AC:" << s.acEnabled
                         << "Auto:" << s.autoEnabled;
                emit climateStatusReceived(s);
            }
            break;

        case 0x1C:
            if (data.size() >= 1){

            int hour   = static_cast<uint8_t>(data[1]);
            int min    = static_cast<uint8_t>(data[2]);
            int sec    = static_cast<uint8_t>(data[3]);
            int day    = static_cast<uint8_t>(data[4]);
            int month  = static_cast<uint8_t>(data[5]);
            int year   = (static_cast<uint8_t>(data[6]) << 8)
                       |  static_cast<uint8_t>(data[7]);
            int format = static_cast<uint8_t>(data[8]); // 0=24h, 1=12h

            emit rtcTimeReceived(hour, min, sec, day, month, year, format);
            }
            break;

        default:
            // Every other response is treated as ACK for the in-flight packet
            qDebug() << "[UART] ACK received for CMD" << Qt::hex << cmd;
            handleAck(cmd);
            break;
        }

        rxBuffer.remove(0, 20);
    }

    emit dataReceived(rxBuffer);
}

void UARTController::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
        qDebug() << "[UART] Error:" << serialPort->errorString();
}
