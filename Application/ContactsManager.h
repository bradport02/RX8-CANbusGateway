#ifndef CONTACTSMANAGER_H
#define CONTACTSMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QSqlDatabase>
#include <QProcess>
#include <QTimer>

class ContactsManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool syncing       READ isSyncing    NOTIFY syncingChanged)
    Q_PROPERTY(int  contactCount  READ contactCount NOTIFY contactsLoaded)
    Q_PROPERTY(QVariantList contacts READ contacts  NOTIFY contactsLoaded)

public:
    explicit ContactsManager(QObject *parent = nullptr);
    ~ContactsManager();

    bool         isSyncing()    const { return m_syncing; }
    int          contactCount() const { return m_contacts.size(); }
    QVariantList contacts()     const { return m_contacts; }

public slots:
    void syncContacts(const QString &deviceAddress);
    void clearContacts();
    Q_INVOKABLE QVariantList search(const QString &query) const;

signals:
    void syncingChanged();
    void contactsLoaded();
    void syncError(const QString &message);

private slots:
    void onSyncFinished(int exitCode);

private:
    void loadFromDatabase();
    void parseAndSaveVCard(const QString &filePath);
    bool openDatabase();
    void attemptSync();
    void cancelSync();          // kill process + reset all state

    QSqlDatabase  m_db;
    QVariantList  m_contacts;
    bool          m_syncing        = false;
    bool          m_retryPending   = false;  // true while retry timer is queued
    QProcess     *m_syncProcess    = nullptr;
    QString       m_deviceAddress;
    int           m_retryCount     = 0;
    static constexpr int MAX_RETRIES = 5;
};

#endif // CONTACTSMANAGER_H
