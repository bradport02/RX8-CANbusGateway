#include "uartController.h"
#include <QDebug>

UARTController::UARTController(QObject *parent)
    : QObject(parent)
{
    serialPort = new QSerialPort(this);

    connect(serialPort, &QSerialPort::readyRead,
            this, &UARTController::handleReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred,
            this, &UARTController::handleError);
}

UARTController::~UARTController()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
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
        qDebug() << "Serial port opened:" << portName;
        emit connectionStatusChanged(true);
        return true;
    } else {
        qDebug() << "Failed to open serial port:" << serialPort->errorString();
        emit connectionStatusChanged(false);
        return false;
    }
}

void UARTController::closePort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        emit connectionStatusChanged(false);
    }
}

// CRC8 calculation (using polynomial 0x07)
quint8 UARTController::calculateCRC8(const QByteArray &data)
{
    quint8 crc = 0x00;

    for (int i = 0; i < data.size(); i++) {
        crc ^= static_cast<quint8>(data[i]);

        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// Pad data to specified length with 0x00
QByteArray UARTController::padData(const QByteArray &data, int length)
{
    QByteArray padded = data;

    // Truncate if too long
    if (padded.size() > length) {
        padded = padded.left(length);
    }

    // Pad with 0x00 if too short
    while (padded.size() < length) {
        padded.append(static_cast<char>(0x00));
    }

    return padded;
}

// Simple send packet function - you just provide CMD and DATA
void UARTController::sendPacket(int cmd, const QString &data)
{
    if (!serialPort->isOpen()) {
        qDebug() << "Serial port not open!";
        return;
    }

    // Prepare the message
    QByteArray message;

    // START byte
    message.append(static_cast<char>(0xAA));

    // CMD byte
    quint8 cmdByte = static_cast<quint8>(cmd);
    message.append(static_cast<char>(cmdByte));

    // Convert string data to bytes and pad to 16 bytes
    QByteArray dataBytes = data.toUtf8();
    QByteArray paddedData = padData(dataBytes, 16);

    // LEN byte (always 16 in your protocol)
    quint8 len = static_cast<quint8>(paddedData.size());
    message.append(static_cast<char>(len));

    // DATA (16 bytes)
    message.append(paddedData);

    // Calculate CRC on CMD + LEN + DATA
    QByteArray crcData;
    crcData.append(static_cast<char>(cmdByte));
    crcData.append(static_cast<char>(len));
    crcData.append(paddedData);

    quint8 crc = calculateCRC8(crcData);

    // CRC byte
    message.append(static_cast<char>(crc));

    // Send the message
    serialPort->write(message);

    // Debug output
    QString debugMsg = "Sent: ";
    for (int i = 0; i < message.size(); i++) {
        debugMsg += QString("0x%1 ").arg(static_cast<quint8>(message[i]), 2, 16, QChar('0')).toUpper();
    }
    qDebug() << debugMsg;
}

void UARTController::handleReadyRead()
{
    QByteArray data = serialPort->readAll();

    // Debug output
    QString debugMsg = "Received: ";
    for (int i = 0; i < data.size(); i++) {
        debugMsg += QString("0x%1 ").arg(static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    qDebug() << debugMsg;

    emit dataReceived(data);
}

void UARTController::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        qDebug() << "Serial port error:" << serialPort->errorString();
    }
}
