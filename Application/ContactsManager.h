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

    bool         isSyncing()    const { return m_syncing; }
    int          contactCount() const { return m_contacts.size(); }
    QVariantList contacts()     const { return m_contacts; }

public slots:
    void syncContacts(const QString &deviceAddress);
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

    QSqlDatabase  m_db;
    QVariantList  m_contacts;
    bool          m_syncing      = false;
    QProcess     *m_syncProcess  = nullptr;
    QString       m_deviceAddress;
};

#endif // CONTACTSMANAGER_H
