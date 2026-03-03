#include "CallManager.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QTimer>
#include <QDebug>

CallManager::CallManager(QObject *parent)
    : QObject(parent)
{
    m_durationTimer = new QTimer(this);
    m_durationTimer->setInterval(1000);
    connect(m_durationTimer, &QTimer::timeout, this, &CallManager::onDurationTick);
}

void CallManager::setModemPath(const QString &path)
{
    if (path.isEmpty()) return;

    m_modemPath = path;

    if (m_vcm) { delete m_vcm; m_vcm = nullptr; }

    QDBusConnection bus = QDBusConnection::systemBus();
    m_vcm = new QDBusInterface(
        "org.ofono", path, "org.ofono.VoiceCallManager", bus, this);

    if (!m_vcm->isValid()) {
        qWarning() << "[Call] VoiceCallManager not ready yet, retrying in 2s...";
        delete m_vcm;
        m_vcm = nullptr;
        QTimer::singleShot(2000, this, [this, path]() {
            retrySetModemPath(path, 1);
        });
        return;
    }

    setupSignals(path);
    qDebug() << "[Call] Modem ready:" << path;
}

void CallManager::retrySetModemPath(const QString &path, int attempt)
{
    if (m_vcm) { delete m_vcm; m_vcm = nullptr; }

    QDBusConnection bus = QDBusConnection::systemBus();
    m_vcm = new QDBusInterface(
        "org.ofono", path, "org.ofono.VoiceCallManager", bus, this);

    if (!m_vcm->isValid()) {
        delete m_vcm;
        m_vcm = nullptr;

        if (attempt < 5) {
            qWarning() << "[Call] Retry" << attempt << "failed, trying again in 2s...";
            QTimer::singleShot(2000, this, [this, path, attempt]() {
                retrySetModemPath(path, attempt + 1);
            });
        } else {
            qWarning() << "[Call] VoiceCallManager failed after 5 attempts.";
        }
        return;
    }

    setupSignals(path);
    qDebug() << "[Call] Modem ready after" << attempt << "retries:" << path;
}

void CallManager::setupSignals(const QString &modemPath)
{
    QDBusConnection bus = QDBusConnection::systemBus();

    bus.connect("org.ofono", modemPath,
                "org.ofono.VoiceCallManager", "CallAdded",
                this, SLOT(onCallAdded(QDBusObjectPath, QVariantMap)));

    bus.connect("org.ofono", modemPath,
                "org.ofono.VoiceCallManager", "CallRemoved",
                this, SLOT(onCallRemoved(QDBusObjectPath)));
}

void CallManager::acceptCall()
{
    if (m_currentCallPath.isEmpty()) return;
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface call("org.ofono", m_currentCallPath, "org.ofono.VoiceCall", bus);
    call.asyncCall("Answer");

    m_callState = CallState::Active;
    m_callDuration = 0;
    m_durationTimer->start();
    emit callStateChanged();
    emit callConnected();
}

void CallManager::declineCall()
{
    if (m_currentCallPath.isEmpty()) return;
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface call("org.ofono", m_currentCallPath, "org.ofono.VoiceCall", bus);
    call.asyncCall("Hangup");
}

void CallManager::hangup()
{
    if (!m_vcm || !m_vcm->isValid()) return;
    m_vcm->asyncCall("HangupAll");
}

void CallManager::dial(const QString &number)
{
    if (!m_vcm || !m_vcm->isValid()) {
        qWarning() << "[Call] Cannot dial — VoiceCallManager not ready";
        return;
    }
    m_callerNumber = number;
    m_callerName.clear();
    m_callState = CallState::Dialling;
    emit callStateChanged();
    qDebug() << "[Call] Dialling" << number;
    m_vcm->asyncCall("Dial", number, "");
}

void CallManager::setMute(bool muted)
{
    if (m_muted == muted) return;
    m_muted = muted;

    // Set mute via oFono CallVolume interface
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface callVol("org.ofono", m_modemPath,
                           "org.ofono.CallVolume", bus);
    if (callVol.isValid()) {
        QDBusInterface props("org.ofono", m_modemPath,
                             "org.freedesktop.DBus.Properties", bus);
        props.asyncCall("Set", "org.ofono.CallVolume", "Muted",
                        QVariant::fromValue(QDBusVariant(muted)));
    }

    emit mutedChanged();
    qDebug() << "[Call] Mute:" << muted;
}

void CallManager::setVolume(int vol)
{
    vol = qBound(0, vol, 100);
    if (m_volume == vol) return;
    m_volume = vol;

    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface props("org.ofono", m_modemPath,
                         "org.freedesktop.DBus.Properties", bus);
    if (props.isValid()) {
        props.asyncCall("Set", "org.ofono.CallVolume", "SpeakerVolume",
                        QVariant::fromValue(QDBusVariant((uchar)vol)));
    }

    emit volumeChanged();
}

void CallManager::sendDtmf(const QString &tone)
{
    if (!m_vcm || !m_vcm->isValid()) return;
    m_vcm->asyncCall("SendTones", tone);
    qDebug() << "[Call] DTMF:" << tone;
}

void CallManager::onCallAdded(const QDBusObjectPath &path, const QVariantMap &props)
{
    m_currentCallPath = path.path();

    QString state  = props.value("State").toString();
    QString lineId = props.value("LineIdentification").toString();
    QString name   = props.value("Name").toString();

    m_callerNumber = lineId;
    m_callerName   = name;

    qDebug() << "[Call] CallAdded" << lineId << "state:" << state;

    QDBusConnection bus = QDBusConnection::systemBus();
    bus.connect("org.ofono", m_currentCallPath,
                "org.freedesktop.DBus.Properties", "PropertiesChanged",
                this, SLOT(onCallPropertiesChanged(QString, QVariantMap, QStringList)));

    if (state == "incoming") {
        m_callState = CallState::Incoming;
        emit callStateChanged();
        emit incomingCall(lineId, name);
    } else if (state == "active") {
        m_callState = CallState::Active;
        m_durationTimer->start();
        emit callStateChanged();
        emit callConnected();
    }
}

void CallManager::onCallRemoved(const QDBusObjectPath &path)
{
    if (path.path() != m_currentCallPath) return;
    teardownCall();
    emit callEnded();
}

void CallManager::onCallPropertiesChanged(
    const QString &,
    const QVariantMap &changed,
    const QStringList &)
{
    if (!changed.contains("State")) return;

    QString state = changed["State"].toString();
    qDebug() << "[Call] State ->" << state;

    if (state == "active") {
        m_callState = CallState::Active;
        m_callDuration = 0;
        m_durationTimer->start();
        emit callStateChanged();
        emit callConnected();
    } else if (state == "disconnected") {
        teardownCall();
        emit callEnded();
    }
}

void CallManager::onDurationTick()
{
    m_callDuration++;
    emit callDurationChanged();
}

void CallManager::teardownCall()
{
    m_durationTimer->stop();
    m_callState = CallState::Idle;
    m_callerName.clear();
    m_callerNumber.clear();
    m_currentCallPath.clear();
    m_callDuration = 0;
    m_muted = false;
    emit callStateChanged();
    emit mutedChanged();
}
