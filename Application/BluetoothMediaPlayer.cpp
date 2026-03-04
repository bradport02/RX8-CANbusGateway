#include "BluetoothMediaPlayer.h"
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusVariant>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTimer>

// ── Helper: fetch a single property via D-Bus Properties.Get ─────────────────
static QVariant getPlayerProperty(const QString &path, const QString &prop)
{
    QDBusMessage req = QDBusMessage::createMethodCall(
        "org.bluez", path,
        "org.freedesktop.DBus.Properties", "Get");
    req << "org.bluez.MediaPlayer1" << prop;
    QDBusMessage reply = QDBusConnection::systemBus().call(req);
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
        return QVariant();
    QDBusVariant v = reply.arguments().first().value<QDBusVariant>();
    return v.variant();
}

BluetoothMediaPlayer::BluetoothMediaPlayer(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<QMap<QString, QDBusVariant>>();
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(1000);
    connect(m_pollTimer, &QTimer::timeout, this, &BluetoothMediaPlayer::pollPosition);
}

void BluetoothMediaPlayer::setPlayerPath(const QString &path)
{
    if (m_playerPath == path) return;
    m_playerPath = path;

    delete m_player;
    m_player = nullptr;
    m_pollTimer->stop();

    if (path.isEmpty()) {
        m_artist.clear(); m_album.clear(); m_title.clear();
        m_duration = 0; m_position = 0; m_status.clear();
        emit trackChanged(); emit positionChanged(); emit statusChanged();
        return;
    }

    tryConnectPlayer(path, 0);
}

void BluetoothMediaPlayer::tryConnectPlayer(const QString &basePath, int attempt)
{
    for (int i = 0; i <= 2; i++) {
        QString tryPath = basePath + "/player" + QString::number(i);
        QDBusInterface *iface = new QDBusInterface("org.bluez", tryPath,
                                                   "org.bluez.MediaPlayer1",
                                                   QDBusConnection::systemBus());
        if (iface->isValid()) {
            delete iface;
            connectToPlayer(tryPath);
            return;
        }
        delete iface;
    }

    if (attempt < 5) {
        qDebug() << "[Media] Player not ready, retry" << attempt + 1 << "in 2s...";
        QTimer::singleShot(2000, this, [this, basePath, attempt]() {
            tryConnectPlayer(basePath, attempt + 1);
        });
    } else {
        qWarning() << "[Media] Could not find MediaPlayer1 at" << basePath;
    }
}

void BluetoothMediaPlayer::connectToPlayer(const QString &path)
{
    qDebug() << "[Media] Connecting to player at" << path;

    m_player = new QDBusInterface("org.bluez", path,
                                  "org.bluez.MediaPlayer1",
                                  QDBusConnection::systemBus(), this);

    if (!m_player->isValid()) {
        qWarning() << "[Media] Invalid player interface at" << path;
        return;
    }

    QDBusConnection::systemBus().connect(
        "org.bluez", path,
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    refreshTrack();
    refreshStatus();
    m_pollTimer->start();
    qDebug() << "[Media] Player connected:" << path;
}

void BluetoothMediaPlayer::refreshTrack()
{
    if (!m_player) return;

    QDBusMessage req = QDBusMessage::createMethodCall(
        "org.bluez", m_player->path(),
        "org.freedesktop.DBus.Properties", "Get");
    req << "org.bluez.MediaPlayer1" << "Track";

    QDBusMessage reply = QDBusConnection::systemBus().call(req);
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty()) {
        qWarning() << "[Media] Failed to get Track property";
        return;
    }

    QDBusVariant dbusVar = qdbus_cast<QDBusVariant>(reply.arguments().first());
    QDBusArgument arg = dbusVar.variant().value<QDBusArgument>();

    QMap<QString, QDBusVariant> rawMap;
    arg >> rawMap;

    QVariantMap track;
    for (auto it = rawMap.cbegin(); it != rawMap.cend(); ++it)
        track.insert(it.key(), it.value().variant());

    if (track.isEmpty()) {
        qWarning() << "[Media] Track map is empty";
        return;
    }

    m_artist   = track.value("Artist",   "").toString();
    m_album    = track.value("Album",    "").toString();
    m_title    = track.value("Title",    "").toString();
    m_duration = track.value("Duration", 0u).toUInt();

    qDebug() << "[Media] Track:" << m_artist << "-" << m_title
             << "(" << m_album << ") duration:" << m_duration << "ms";

    emit trackChanged();
}

void BluetoothMediaPlayer::refreshStatus()
{
    if (!m_player) return;
    QString s = getPlayerProperty(m_player->path(), "Status").toString();
    if (s.isEmpty()) return;
    m_status = s;
    emit statusChanged();

    if (m_status == "playing")
        m_pollTimer->start();
    else
        m_pollTimer->stop();
}

void BluetoothMediaPlayer::refreshPosition()
{
    if (!m_player) return;
    QVariant v = getPlayerProperty(m_player->path(), "Position");
    if (!v.isValid()) return;
    quint32 pos = v.toUInt();
    if (pos != m_position) {
        m_position = pos;
        emit positionChanged();
    }
}

void BluetoothMediaPlayer::pollPosition()
{
    refreshPosition();
}

void BluetoothMediaPlayer::onPropertiesChanged(const QString &interface,
                                               const QVariantMap &changed,
                                               const QStringList &)
{
    if (interface != "org.bluez.MediaPlayer1") return;

    if (changed.contains("Track"))
        refreshTrack();

    if (changed.contains("Status")) {
        m_status = changed.value("Status").toString();
        emit statusChanged();
        if (m_status == "playing")
            m_pollTimer->start();
        else
            m_pollTimer->stop();
    }

    if (changed.contains("Position")) {
        m_position = changed.value("Position").toUInt();
        emit positionChanged();
    }
}

void BluetoothMediaPlayer::play()     { if (m_player) m_player->call("Play");     }
void BluetoothMediaPlayer::pause()    { if (m_player) m_player->call("Pause");    }
void BluetoothMediaPlayer::previous() { if (m_player) m_player->call("Previous"); }
void BluetoothMediaPlayer::next()     { if (m_player) m_player->call("Next");     }
