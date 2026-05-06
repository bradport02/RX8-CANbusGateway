#ifndef UARTCONTROLLER_H
#define UARTCONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QMetaType>
#include <QTimer>
#include <QQueue>

// ── Climate status — top-level for Q_GADGET support ──────────────────────────
class ClimateStatus
{
    Q_GADGET

    Q_PROPERTY(int  tempTens    MEMBER tempTens)
    Q_PROPERTY(int  tempUnits   MEMBER tempUnits)
    Q_PROPERTY(int  tempDecimal MEMBER tempDecimal)
    Q_PROPERTY(int  fanSpeed    MEMBER fanSpeed)
    Q_PROPERTY(int  sysMode     MEMBER sysMode)
    Q_PROPERTY(int  ventMode    MEMBER ventMode)
    Q_PROPERTY(int  circMode    MEMBER circMode)
    Q_PROPERTY(bool autoEnabled MEMBER autoEnabled)
    Q_PROPERTY(bool acEnabled   MEMBER acEnabled)
    Q_PROPERTY(bool ecoEnabled  MEMBER ecoEnabled)
    Q_PROPERTY(bool demistFront MEMBER demistFront)
    Q_PROPERTY(bool demistRear  MEMBER demistRear)

public:
    int  tempTens    = 0;
    int  tempUnits   = 0;
    int  tempDecimal = 0;
    int  fanSpeed    = 0;
    int  sysMode     = 0;
    int  ventMode    = 0;
    int  circMode    = 0;
    bool autoEnabled = false;
    bool acEnabled   = false;
    bool ecoEnabled  = false;
    bool demistFront = false;
    bool demistRear  = false;
};

Q_DECLARE_METATYPE(ClimateStatus)

// ── UART Controller ───────────────────────────────────────────────────────────
class UARTController : public QObject
{
    Q_OBJECT

public:
    explicit UARTController(QObject *parent = nullptr);
    ~UARTController();

    Q_INVOKABLE bool openPort(const QString &portName);
    Q_INVOKABLE void closePort();

    // Single byte payload
    Q_INVOKABLE void send(quint8 cmd, quint8 data);

    // Multi-byte payload
    Q_INVOKABLE void sendData(quint8 cmd, const QByteArray &data);

    // LCD text helper
    Q_INVOKABLE void sendLCDText(const QString &text);

    // Temperature BCD: 18.5 → {0x01, 0x08, 0x05}
    Q_INVOKABLE void sendTemperature(quint8 cmd, float temperature);

    // Audio settings: 5 signed values (-5 to +5), offset +5 per byte
    Q_INVOKABLE void sendAudioSettings(int bass, int treble, int mids, int fader, int balance);

    //Send ambient lighting data
    Q_INVOKABLE void sendAmbient(int r, int g, int b, int brightness, bool on);

    //Send Time Data
    Q_INVOKABLE void sendTime(int hour, int min, int second, int day, int month, int year, int format);

public slots:
    void onVolumeChanged(int volume);

signals:
    void dataReceived(QByteArray data);
    void connectionStatusChanged(bool connected);
    void climateStatusReceived(ClimateStatus status);
    void rtcTimeReceived(int hour, int min, int sec, int day,  int month, int year, int format);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
    void onAckTimeout();

private:
    // ── Serial port ──────────────────────────────────────────────────────────
    QSerialPort *serialPort;
    QByteArray   rxBuffer;

    // ── Packet builder / debug ────────────────────────────────────────────────
    quint8     calculateCRC8(const QByteArray &data);
    QByteArray buildPacket(quint8 cmd, const QByteArray &data);
    void       debugPacket(const QByteArray &packet);

    // ── Unified send queue ────────────────────────────────────────────────────
    struct PendingPacket {
        quint8     cmd  = 0;
        QByteArray data;
    };

    static const int MAX_RETRIES = 0;

    QQueue<PendingPacket> sendQueue;
    PendingPacket         currentPacket;
    bool                  inFlight   = false;
    int                   retryCount = 0;
    QTimer               *ackTimer   = nullptr;

    void enqueue(quint8 cmd, const QByteArray &data);
    void sendNext();
    void handleAck(quint8 cmd);
};

#endif // UARTCONTROLLER_H
