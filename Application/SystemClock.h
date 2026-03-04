#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H

#include <QObject>
#include <QProcess>
#include <QDateTime>
#include <QSettings>
#include <QDebug>

class SystemClock : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool use24h READ use24h WRITE setUse24h NOTIFY use24hChanged)

public:
    explicit SystemClock(QObject *parent = nullptr) : QObject(parent) {
        QSettings settings("headunit", "clock");
        m_use24h = settings.value("use24h", true).toBool();
    }

    bool use24h() const { return m_use24h; }

    Q_INVOKABLE void setUse24h(bool val) {
        if (m_use24h == val) return;
        m_use24h = val;
        QSettings settings("headunit", "clock");
        settings.setValue("use24h", val);
        qDebug() << "[Clock] Format:" << (val ? "24h" : "12h");
        emit use24hChanged();
    }

    // Keep old name working from ClockSettingsPage
    Q_INVOKABLE void setFormat(bool val) { setUse24h(val); }
    Q_INVOKABLE bool load24h()           { return m_use24h; }

    Q_INVOKABLE void setDateTime(int year, int month, int day,
                                 int hour, int minute, int second) {
        QString dateStr = QString("%1-%2-%3")
        .arg(year)
            .arg(month,  2, 10, QChar('0'))
            .arg(day,    2, 10, QChar('0'));
        QString timeStr = QString("%1:%2:%3")
                              .arg(hour,   2, 10, QChar('0'))
                              .arg(minute, 2, 10, QChar('0'))
                              .arg(second, 2, 10, QChar('0'));

        QProcess::execute("bash", {"-c", "timedatectl set-ntp false"});
        QString cmd = QString("timedatectl set-time '%1 %2'").arg(dateStr, timeStr);
        qDebug() << "[Clock] Setting datetime:" << cmd;
        QProcess::startDetached("bash", {"-c", cmd});
    }

    Q_INVOKABLE void setTime(int hour, int minute, int second) {
        QDateTime now = QDateTime::currentDateTime();
        setDateTime(now.date().year(), now.date().month(), now.date().day(),
                    hour, minute, second);
    }

signals:
    void use24hChanged();

private:
    bool m_use24h = true;
};

#endif
