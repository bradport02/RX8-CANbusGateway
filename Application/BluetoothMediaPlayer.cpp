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

    // ImgHandle is in Track dict; ObexPort is a player-level property
    QString imgHandle = track.value("ImgHandle", "").toString();
    if (!imgHandle.isEmpty()) {
        QVariant obexVar = getPlayerProperty(m_player->path(), "ObexPort");
        quint16 obexPort = obexVar.isValid() ? (quint16)obexVar.toUInt() : 0;
        qDebug() << "[Media] ImgHandle:" << imgHandle << "ObexPort:" << obexPort;
        if (obexPort > 0)
            fetchArtwork(imgHandle, obexPort);
        else
            qWarning() << "[Media] ObexPort not available - skipping artwork";
    }

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

void BluetoothMediaPlayer::fetchArtwork(const QString &imgHandle, quint16 obexPort)
{
    const QString scriptPath = "/tmp/headunit_art_fetch.py";
    const QString outPath    = "/tmp/headunit_artwork.jpg";

    // Extract BT address from player path e.g. /org/bluez/hci0/dev_DC_53_92_33_50_56/player0
    QString playerPath = m_player->path();
    // Strip everything up to dev_ prefix, then strip /playerN suffix
    QString addr = playerPath;
    int devIdx = addr.indexOf("/dev_");
    if (devIdx >= 0) addr = addr.mid(devIdx + 5); // skip "/dev_"
    int slashIdx = addr.indexOf('/');
    if (slashIdx >= 0) addr = addr.left(slashIdx); // strip /player0
    addr.replace("_", ":");

    QFile script(scriptPath);
    if (!script.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;

    QTextStream s(&script);
    s << QString(R"(
import socket, struct, os, sys

addr   = '%1'
port   = %2
handle = '%3'
out    = '%4'

def obex_connect():
    sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    sock.settimeout(8)
    sock.connect((addr, port))
    pkt = bytes([0x80, 0x00, 0x07, 0x10, 0x00, 0xFF, 0xFF])
    sock.send(pkt)
    resp = sock.recv(64)
    if resp[0] != 0xA0:
        raise Exception('OBEX connect failed: ' + hex(resp[0]))
    return sock

try:
    sock = obex_connect()

    img_type = b'x-bt/img-thumbnail\x00'
    handle_b = handle.encode() + b'\x00'

    type_hdr   = bytes([0x42]) + struct.pack('>H', 3 + len(img_type)) + img_type
    handle_hdr = bytes([0x30]) + struct.pack('>H', 3 + len(handle_b)) + handle_b

    body = type_hdr + handle_hdr
    pkt  = bytes([0x83]) + struct.pack('>H', 3 + len(body)) + body
    sock.send(pkt)

    data = b''
    while True:
        resp = sock.recv(4096)
        if len(resp) < 3:
            break
        code    = resp[0]
        payload = resp[3:]
        i = 0
        while i < len(payload) - 2:
            hid  = payload[i]
            hlen = struct.unpack('>H', payload[i+1:i+3])[0]
            if hid in (0x48, 0x49):
                data += payload[i+3:i+hlen]
            i += max(hlen, 1)
        if code == 0xA0:
            break
        if code != 0x90:
            break

    sock.close()
    if data:
        with open(out, 'wb') as f:
            f.write(data)
        print('OK:' + out)
    else:
        print('ERROR: no image data received', file=sys.stderr)
        sys.exit(1)
except Exception as e:
    print('ERROR:' + str(e), file=sys.stderr)
    sys.exit(1)
)").arg(addr).arg(obexPort).arg(imgHandle).arg(outPath);
    script.close();

    QProcess *proc = new QProcess(this);
    proc->setProgram("python3");
    proc->setArguments({scriptPath});
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, outPath](int code, QProcess::ExitStatus) {
                if (code == 0 && QFile::exists(outPath)) {
                    m_artworkPath = outPath;
                    qDebug() << "[Media] Artwork fetched:" << outPath;
                    emit artworkChanged();
                } else {
                    qWarning() << "[Media] Artwork fetch failed:"
                               << proc->readAllStandardError().trimmed();
                }
                proc->deleteLater();
            });
    proc->start();
    qDebug() << "[Media] Fetching artwork handle:" << imgHandle << "port:" << obexPort;
}

void BluetoothMediaPlayer::play()     { if (m_player) m_player->call("Play");     }
void BluetoothMediaPlayer::pause()    { if (m_player) m_player->call("Pause");    }
void BluetoothMediaPlayer::previous() { if (m_player) m_player->call("Previous"); }
void BluetoothMediaPlayer::next()     { if (m_player) m_player->call("Next");     }
