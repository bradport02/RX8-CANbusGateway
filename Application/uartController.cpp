#include "uartController.h"
#include <QDebug>
#include <cmath>

UARTController::UARTController(QObject *parent) : QObject(parent)
{
    serialPort = new QSerialPort(this);
    connect(serialPort, &QSerialPort::readyRead,     this, &UARTController::handleReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &UARTController::handleError);
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
// Frame: 0xAA | CMD | LEN | DATA(padded to 16) | CRC8
// LEN  = actual data bytes (not padded size)
// CRC8 = crc8(CMD + LEN + padded DATA)
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

// ── Transmit ──────────────────────────────────────────────────────────────────
static void debugPacket(const QByteArray &packet)
{
    quint8 cmd = static_cast<quint8>(packet[1]);
    quint8 len = static_cast<quint8>(packet[2]);
    QString dbg = QString("[UART] TX CMD=0x%1 LEN=%2 | ")
                      .arg(cmd, 2, 16, QChar('0')).toUpper().arg(len);
    for (quint8 b : packet)
        dbg += QString("0x%1 ").arg(b, 2, 16, QChar('0')).toUpper();
    qDebug() << dbg;
}

// Single byte — covers most climate/audio/lighting commands
void UARTController::send(quint8 cmd, quint8 data)
{
    QByteArray d;
    d.append(static_cast<char>(data));
    QByteArray packet = buildPacket(cmd, d);
    serialPort->write(packet);
    debugPacket(packet);
}

// Multi-byte — caller provides exact payload bytes
void UARTController::sendData(quint8 cmd, const QByteArray &data)
{
    QByteArray packet = buildPacket(cmd, data);
    serialPort->write(packet);
    debugPacket(packet);
}

// ── Special encodings ─────────────────────────────────────────────────────────

// Temperature as BCD: 18.5 → {0x01, 0x08, 0x05}
void UARTController::sendTemperature(quint8 cmd, float temperature)
{
    temperature = qBound(15.0f, temperature, 32.0f);
    temperature = roundf(temperature * 2.0f) / 2.0f;

    int t = static_cast<int>(roundf(temperature * 10.0f));
    QByteArray data;
    data.append(static_cast<char>((t / 100) % 10));  // tens
    data.append(static_cast<char>((t / 10)  % 10));  // units
    data.append(static_cast<char>( t        % 10));  // tenths
    sendData(cmd, data);
}

// Audio settings: offset -5..+5 by +5 so each fits in one unsigned byte (0-10)
// Receiver subtracts 5 to recover signed value
void UARTController::sendAudioSettings(int bass, int treble, int mids, int fader, int balance)
{
    QByteArray data;
    data.append(static_cast<char>(bass    + 5));
    data.append(static_cast<char>(treble  + 5));
    data.append(static_cast<char>(mids    + 5));
    data.append(static_cast<char>(fader   + 5));
    data.append(static_cast<char>(balance + 5));
    sendData(0x14, data);
}

// ── RX ────────────────────────────────────────────────────────────────────────
void UARTController::handleReadyRead()
{
    QByteArray data = serialPort->readAll();
    QString dbg = "[UART] RX | ";
    for (quint8 b : data)
        dbg += QString("0x%1 ").arg(b, 2, 16, QChar('0')).toUpper();
    qDebug() << dbg;
    emit dataReceived(data);
}

void UARTController::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
        qDebug() << "[UART] Error:" << serialPort->errorString();
}
