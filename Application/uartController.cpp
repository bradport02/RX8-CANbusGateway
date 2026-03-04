#include "uartController.h"
#include <QDebug>
#include <cmath>

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

QByteArray UARTController::padData(const QByteArray &data, int length)
{
    QByteArray padded = data;
    if (padded.size() > length) {
        padded = padded.left(length);
    }
    while (padded.size() < length) {
        padded.append(static_cast<char>(0x00));
    }
    return padded;
}

void UARTController::sendPacket(int cmd, const QString &data)
{
    if (!serialPort->isOpen()) {
        qDebug() << "Serial port not open!";
        return;
    }

    QByteArray message;
    message.append(static_cast<char>(0xAA));

    quint8 cmdByte = static_cast<quint8>(cmd);
    message.append(static_cast<char>(cmdByte));

    QByteArray dataBytes = data.toUtf8();
    QByteArray paddedData = padData(dataBytes, 16);

    quint8 len = static_cast<quint8>(paddedData.size());
    message.append(static_cast<char>(len));
    message.append(paddedData);

    QByteArray crcData;
    crcData.append(static_cast<char>(cmdByte));
    crcData.append(static_cast<char>(len));
    crcData.append(paddedData);

    quint8 crc = calculateCRC8(crcData);
    message.append(static_cast<char>(crc));

    serialPort->write(message);

    QString debugMsg = "Sent: ";
    for (int i = 0; i < message.size(); i++) {
        debugMsg += QString("0x%1 ").arg(static_cast<quint8>(message[i]), 2, 16, QChar('0')).toUpper();
    }
    qDebug() << debugMsg;
}

// Send temperature as BCD — one digit per byte, MSB to LSB
// Example: 18.5 -> 0x01, 0x08, 0x05
// Example: 22.0 -> 0x02, 0x02, 0x00
// Decimal point is always implicit between byte 2 and byte 3
void UARTController::sendTemperature(int cmd, float temperature)
{
    if (!serialPort->isOpen()) {
        qDebug() << "Serial port not open!";
        return;
    }

    // Clamp to valid range and round to nearest 0.5
    temperature = qBound(15.0f, temperature, 32.0f);
    temperature = roundf(temperature * 2.0f) / 2.0f;

    // Split into 3 BCD digits: tens, units, tenths
    // Multiply by 10 to eliminate decimal, then extract each digit
    int total_tenths = static_cast<int>(roundf(temperature * 10.0f));
    quint8 digit_tens   = static_cast<quint8>((total_tenths / 100) % 10);
    quint8 digit_units  = static_cast<quint8>((total_tenths / 10)  % 10);
    quint8 digit_tenths = static_cast<quint8>( total_tenths        % 10);

    qDebug() << QString("[UART] Temp BCD: %1 C -> 0x0%2, 0x0%3, 0x0%4")
                    .arg(temperature, 0, 'f', 1)
                    .arg(digit_tens)
                    .arg(digit_units)
                    .arg(digit_tenths);

    // First 3 bytes are BCD digits, remaining 13 bytes padded with 0x00
    QByteArray dataBytes;
    dataBytes.append(static_cast<char>(digit_tens));
    dataBytes.append(static_cast<char>(digit_units));
    dataBytes.append(static_cast<char>(digit_tenths));
    QByteArray paddedData = padData(dataBytes, 16);

    // Build packet
    QByteArray message;
    message.append(static_cast<char>(0xAA));

    quint8 cmdByte = static_cast<quint8>(cmd);
    message.append(static_cast<char>(cmdByte));

    quint8 len = static_cast<quint8>(paddedData.size());
    message.append(static_cast<char>(len));
    message.append(paddedData);

    QByteArray crcData;
    crcData.append(static_cast<char>(cmdByte));
    crcData.append(static_cast<char>(len));
    crcData.append(paddedData);
    message.append(static_cast<char>(calculateCRC8(crcData)));

    serialPort->write(message);

    QString debugMsg = "Sent temp packet: ";
    for (int i = 0; i < message.size(); i++) {
        debugMsg += QString("0x%1 ").arg(static_cast<quint8>(message[i]), 2, 16, QChar('0')).toUpper();
    }
    qDebug() << debugMsg;
}

void UARTController::handleReadyRead()
{
    QByteArray data = serialPort->readAll();

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
