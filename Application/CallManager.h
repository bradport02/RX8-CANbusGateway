#ifndef CALLMANAGER_H
#define CALLMANAGER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QVariantMap>
#include <QTimer>

class CallManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(CallState callState   READ callState   NOTIFY callStateChanged)
    Q_PROPERTY(QString   callerName  READ callerName  NOTIFY callStateChanged)
    Q_PROPERTY(QString   callerNumber READ callerNumber NOTIFY callStateChanged)
    Q_PROPERTY(int       callDuration READ callDuration NOTIFY callDurationChanged)
    Q_PROPERTY(bool      muted       READ isMuted     NOTIFY mutedChanged)
    Q_PROPERTY(int       volume      READ volume      NOTIFY volumeChanged)

public:
    enum class CallState { Idle, Incoming, Active, Dialling };
    Q_ENUM(CallState)

    explicit CallManager(QObject *parent = nullptr);

    CallState callState()    const { return m_callState; }
    QString   callerName()   const { return m_callerName; }
    QString   callerNumber() const { return m_callerNumber; }
    int       callDuration() const { return m_callDuration; }
    bool      isMuted()      const { return m_muted; }
    int       volume()       const { return m_volume; }

    void setModemPath(const QString &path);

public slots:
    void acceptCall();
    void declineCall();
    void hangup();
    void dial(const QString &number);
    void setMute(bool muted);
    void setVolume(int volume);     // 0-100
    void sendDtmf(const QString &tone);  // for dial pad during call

signals:
    void callStateChanged();
    void callDurationChanged();
    void mutedChanged();
    void volumeChanged();
    void incomingCall(const QString &number, const QString &name);
    void callConnected();
    void callEnded();

private slots:
    void onCallAdded(const QDBusObjectPath &path, const QVariantMap &props);
    void onCallRemoved(const QDBusObjectPath &path);
    void onCallPropertiesChanged(const QString &iface,
                                 const QVariantMap &changed,
                                 const QStringList &invalidated);
    void onDurationTick();

private:
    void setupSignals(const QString &modemPath);
    void retrySetModemPath(const QString &path, int attempt);
    void teardownCall();

    QDBusInterface *m_vcm             = nullptr;
    QString         m_currentCallPath;
    QString         m_modemPath;

    CallState  m_callState    = CallState::Idle;
    QString    m_callerName;
    QString    m_callerNumber;
    int        m_callDuration = 0;
    bool       m_muted        = false;
    int        m_volume       = 80;
    QTimer    *m_durationTimer = nullptr;
};

#endif // CALLMANAGER_H
