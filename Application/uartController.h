#ifndef UARTCONTROLLER_H
#define UARTCONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class UARTController : public QObject
{
    Q_OBJECT
public:
    explicit UARTController(QObject *parent = nullptr);
    ~UARTController();

    Q_INVOKABLE bool openPort(const QString &portName);
    Q_INVOKABLE void closePort();

    // Simple packet sending - just CMD and DATA
    Q_INVOKABLE void sendPacket(int cmd, const QString &data);
    Q_INVOKABLE void sendTemperature(int cmd, float temperature);

signals:
    void dataReceived(QByteArray data);
    void connectionStatusChanged(bool connected);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serialPort;

    // Protocol functions
    quint8 calculateCRC8(const QByteArray &data);
    QByteArray padData(const QByteArray &data, int length = 16);
};

#endif
