#ifndef CARPLAYCONTROLLER_H
#define CARPLAYCONTROLLER_H

#include <QObject>
#include <QProcess>

class CarPlayController : public QObject
{
    Q_OBJECT
public:
    explicit CarPlayController(QObject *parent = nullptr);
    ~CarPlayController();

    Q_INVOKABLE void launchCarPlay();
    Q_INVOKABLE void stopCarPlay();

signals:
    void carplayExited(int exitCode);
    void shouldReturnHome();

private slots:
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessError(QProcess::ProcessError error);

private:
    QProcess *carplayProcess;
};

#endif
