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

    // ── Primary send functions ────────────────────────────────────────────────
    // Single byte payload  — covers most climate/audio/lighting commands
    Q_INVOKABLE void send(quint8 cmd, quint8 data);

    // Multi-byte payload — caller builds QByteArray with exact data bytes
    Q_INVOKABLE void sendData(quint8 cmd, const QByteArray &data);

    // ── Helpers for special encoding ──────────────────────────────────────────
    // Temperature BCD: 18.5 → {0x01, 0x08, 0x05}
    Q_INVOKABLE void sendTemperature(quint8 cmd, float temperature);

    // Audio settings: 5 signed values (-5 to +5), offset +5 per byte
    Q_INVOKABLE void sendAudioSettings(int bass, int treble, int mids, int fader, int balance);

public slots:
    void onVolumeChanged(int volume) {
        //qDebug() << "[Volume] Sending volume:" << volume;
        send(0x0E, static_cast<quint8>(qBound(0, volume, 30)));
    }

signals:
    void dataReceived(QByteArray data);
    void connectionStatusChanged(bool connected);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serialPort;
    quint8       calculateCRC8(const QByteArray &data);
    QByteArray   buildPacket(quint8 cmd, const QByteArray &data);
};

#endif
