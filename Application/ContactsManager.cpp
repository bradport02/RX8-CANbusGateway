#include "ContactsManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QProcessEnvironment>
#include <unistd.h>

ContactsManager::ContactsManager(QObject *parent)
    : QObject(parent)
{
    // Open DB and clear any stale contacts from previous session
    if (openDatabase()) {
        QSqlDatabase db = QSqlDatabase::database("contacts");
        QSqlQuery q(db);
        q.exec("DELETE FROM contacts");
        qDebug() << "[Contacts] Cleared stale contacts from previous session";
    }
}

bool ContactsManager::openDatabase()
{
    QString dbDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbDir);

    m_db = QSqlDatabase::addDatabase("QSQLITE", "contacts");
    m_db.setDatabaseName(dbDir + "/contacts.db");

    if (!m_db.open()) {
        qWarning() << "[Contacts] DB open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS contacts (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT NOT NULL,
            phone_type  TEXT,
            number      TEXT NOT NULL
        )
    )");

    return true;
}

void ContactsManager::loadFromDatabase()
{
    m_contacts.clear();

    QSqlQuery q("SELECT name, phone_type, number FROM contacts "
                "ORDER BY name COLLATE NOCASE", m_db);

    while (q.next()) {
        QVariantMap entry;
        entry["name"]       = q.value(0).toString();
        entry["phone_type"] = q.value(1).toString();
        entry["number"]     = q.value(2).toString();

        // Build initials for avatar circle in QML
        QString name = q.value(0).toString();
        QStringList parts = name.split(' ', Qt::SkipEmptyParts);
        QString initials;
        for (int i = 0; i < qMin(2, parts.size()); ++i)
            initials += parts[i][0].toUpper();
        entry["initials"] = initials.isEmpty() ? "?" : initials;

        m_contacts.append(entry);
    }

    emit contactsLoaded();
    qDebug() << "[Contacts] Loaded" << m_contacts.size() << "contacts";
}

void ContactsManager::syncContacts(const QString &deviceAddress)
{
    // Cancel any existing sync/retry chain before starting fresh
    if (m_syncing || m_retryPending)
        cancelSync();

    m_deviceAddress = deviceAddress;
    m_retryCount    = 0;
    attemptSync();
}

void ContactsManager::attemptSync()
{
    m_retryPending = false;
    m_syncing      = true;
    emit syncingChanged();

    // Write the obexd PBAP pull script to a temp file
    const QString scriptPath = "/tmp/headunit_pbap_sync.py";
    QFile script(scriptPath);
    if (script.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream s(&script);
        s << R"(
import dbus, time, os, sys

addr = sys.argv[1]
bus  = dbus.SessionBus()

manager = dbus.Interface(
    bus.get_object('org.bluez.obex', '/org/bluez/obex'),
    'org.bluez.obex.Client1'
)

try:
    session = manager.CreateSession(addr, {'Target': dbus.String('pbap')})
    pb = dbus.Interface(bus.get_object('org.bluez.obex', session),
                        'org.bluez.obex.PhonebookAccess1')

    # Select the main phonebook on the phone's internal storage
    pb.Select('int', 'pb')

    # Get total contact count so we know how many to request
    try:
        size = pb.GetSize()
        print('INFO: Phonebook size = ' + str(size), file=sys.stderr)
    except Exception:
        size = 65535  # fallback if GetSize not supported

    # PullAll with explicit MaxListCount to get all contacts in one shot
    transfer, props = pb.PullAll('', {
        'Format':          dbus.String('vcard30'),
        'MaxListCount':    dbus.UInt16(min(int(size), 65535)),
        'ListStartOffset': dbus.UInt16(0),
        'Fields':          dbus.Array(['VERSION','FN','TEL','N'], signature='s'),
    })
    fname = str(props.get('Filename', ''))

    # Wait up to 60s for the transfer file to appear and be non-empty
    for _ in range(60):
        time.sleep(1)
        if os.path.exists(fname) and os.path.getsize(fname) > 0:
            break

    if os.path.exists(fname):
        with open(fname, 'r', errors='ignore') as f:
            print(f.read(), end='')
        os.remove(fname)
    else:
        print('ERROR: Transfer file never appeared: ' + fname, file=sys.stderr)
        sys.exit(1)

    manager.RemoveSession(session)
except Exception as e:
    print('ERROR:' + str(e), file=sys.stderr)
    sys.exit(1)
)";
        script.close();
    }

    // Pass the D-Bus session bus address explicitly so the Python script
    // can reach obexd — QProcess doesn't inherit it from the environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString dbusAddr = qEnvironmentVariable("DBUS_SESSION_BUS_ADDRESS");
    if (dbusAddr.isEmpty()) {
        // Fallback: standard systemd user bus path
        dbusAddr = QString("unix:path=/run/user/%1/bus").arg(getuid());
    }
    env.insert("DBUS_SESSION_BUS_ADDRESS", dbusAddr);

    m_syncProcess = new QProcess(this);
    m_syncProcess->setProcessEnvironment(env);
    m_syncProcess->setProgram("python3");
    m_syncProcess->setArguments({scriptPath, m_deviceAddress});

    connect(m_syncProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) { onSyncFinished(code); });

    m_syncProcess->start();
}

void ContactsManager::onSyncFinished(int exitCode)
{
    if (exitCode != 0) {
        QString err = m_syncProcess->readAllStandardError();
        qWarning() << "[Contacts] Sync failed:" << err.trimmed();

        m_syncProcess->deleteLater();
        m_syncProcess = nullptr;
        m_syncing = false;
        emit syncingChanged();

        // iOS needs time after HFP connects before PBAP is ready — retry
        if (m_retryCount < MAX_RETRIES) {
            m_retryCount++;
            int delayMs = m_retryCount * 5000;  // 5s, 10s, 15s, 20s, 25s
            qDebug() << "[Contacts] Retry" << m_retryCount << "of" << MAX_RETRIES
                     << "in" << delayMs / 1000 << "seconds...";
            m_retryPending = true;
            QTimer::singleShot(delayMs, this, [this]() {
                if (m_retryPending) {
                    m_retryPending = false;
                    attemptSync();
                }
            });
        } else {
            qWarning() << "[Contacts] All retries exhausted — contacts unavailable";
            emit syncError("Could not sync contacts after " +
                           QString::number(MAX_RETRIES) + " attempts");
        }
        return;
    } else {
        QString vcf = m_syncProcess->readAllStandardOutput();
        if (!vcf.isEmpty()) {
            const QString tmp = "/tmp/headunit_contacts.vcf";
            QFile f(tmp);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                f.write(vcf.toUtf8());
                f.close();
            }
            parseAndSaveVCard(tmp);
            QFile::remove(tmp);
        }
    }

    m_syncProcess->deleteLater();
    m_syncProcess = nullptr;
    m_syncing = false;
    emit syncingChanged();
}

void ContactsManager::parseAndSaveVCard(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream stream(&f);
    QString content = stream.readAll();
    f.close();

    QSqlQuery del(m_db);
    del.exec("DELETE FROM contacts");

    QSqlQuery ins(m_db);
    ins.prepare("INSERT INTO contacts (name, phone_type, number) VALUES (?,?,?)");

    QStringList cards = content.split("BEGIN:VCARD", Qt::SkipEmptyParts);
    int saved = 0;

    for (const QString &card : cards) {
        QString name;
        QStringList phones;
        QStringList types;

        for (QString line : card.split('\n')) {
            line = line.trimmed();

            if (line.startsWith("FN"))
                name = line.section(':', 1).trimmed();

            if (line.startsWith("TEL")) {
                QString type = "CELL";
                if (line.contains("HOME", Qt::CaseInsensitive))      type = "HOME";
                else if (line.contains("WORK", Qt::CaseInsensitive)) type = "WORK";
                QString number = line.section(':', -1).trimmed();
                if (!number.isEmpty()) {
                    phones << number;
                    types  << type;
                }
            }
        }

        if (name.isEmpty() || phones.isEmpty()) continue;

        for (int i = 0; i < phones.size(); ++i) {
            ins.bindValue(0, name);
            ins.bindValue(1, types[i]);
            ins.bindValue(2, phones[i]);
            if (ins.exec()) saved++;
        }
    }

    qDebug() << "[Contacts] Saved" << saved << "entries";
    loadFromDatabase();
}

ContactsManager::~ContactsManager()
{
    cancelSync();
}

void ContactsManager::cancelSync()
{
    // Kill any running process immediately
    if (m_syncProcess) {
        m_syncProcess->kill();
        m_syncProcess->waitForFinished(500);
        m_syncProcess->deleteLater();
        m_syncProcess = nullptr;
    }
    m_syncing      = false;
    m_retryPending = false;
    m_retryCount   = 0;
    emit syncingChanged();
}


void ContactsManager::clearContacts()
{
    // Phone disconnected — kill any in-progress sync or pending retry
    cancelSync();
    m_contacts.clear();
    emit contactsLoaded();
    qDebug() << "[Contacts] Cleared (phone disconnected)";
}

QVariantList ContactsManager::search(const QString &query) const
{
    if (query.isEmpty()) return m_contacts;

    QVariantList result;
    const QString lower = query.toLower();
    for (const QVariant &v : m_contacts) {
        QVariantMap m = v.toMap();
        if (m["name"].toString().toLower().contains(lower) ||
            m["number"].toString().contains(lower))
            result.append(m);
    }
    return result;
}
