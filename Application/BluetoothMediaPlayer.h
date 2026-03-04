#ifndef BLUETOOTHMEDIAPLAYER_H
#define BLUETOOTHMEDIAPLAYER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QTimer>

class BluetoothMediaPlayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString artist      READ artist      NOTIFY trackChanged)
    Q_PROPERTY(QString album       READ album       NOTIFY trackChanged)
    Q_PROPERTY(QString title       READ title       NOTIFY trackChanged)
    Q_PROPERTY(quint32 duration    READ duration    NOTIFY trackChanged)
    Q_PROPERTY(quint32 position    READ position    NOTIFY positionChanged)
    Q_PROPERTY(QString status      READ status      NOTIFY statusChanged)
    Q_PROPERTY(QString artworkPath READ artworkPath NOTIFY artworkChanged)

public:
    explicit BluetoothMediaPlayer(QObject *parent = nullptr);

    QString artist()      const { return m_artist; }
    QString album()       const { return m_album; }
    QString title()       const { return m_title; }
    quint32 duration()    const { return m_duration; }
    quint32 position()    const { return m_position; }
    QString status()      const { return m_status; }
    QString artworkPath() const { return m_artworkPath; }

public slots:
    void setPlayerPath(const QString &path);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void next();

signals:
    void trackChanged();
    void positionChanged();
    void statusChanged();
    void artworkChanged();

private slots:
    void pollPosition();
    void onPropertiesChanged(const QString &interface,
                             const QVariantMap &changed,
                             const QStringList &invalidated);

private:
    void tryConnectPlayer(const QString &basePath, int attempt);
    void fetchArtwork(const QString &imgHandle, quint16 obexPort);
    void connectToPlayer(const QString &path);
    void refreshTrack();
    void refreshPosition();
    void refreshStatus();

    QDBusInterface *m_player    = nullptr;
    QTimer         *m_pollTimer = nullptr;

    QString  m_artist;
    QString  m_album;
    QString  m_title;
    quint32  m_duration    = 0;
    quint32  m_position    = 0;
    QString  m_status;
    QString  m_artworkPath;
    QString  m_playerPath;
};

#endif
