#include "carplayController.h"
#include <QDebug>

CarPlayController::CarPlayController(QObject *parent)
    : QObject(parent)
{
    carplayProcess = new QProcess(this);

    connect(carplayProcess, &QProcess::finished,
            this, &CarPlayController::handleProcessFinished);
    connect(carplayProcess, &QProcess::errorOccurred,
            this, &CarPlayController::handleProcessError);
}

CarPlayController::~CarPlayController()
{
    if (carplayProcess->state() == QProcess::Running) {
        carplayProcess->terminate();
        carplayProcess->waitForFinished(3000);
    }
}

void CarPlayController::launchCarPlay()
{
    if (carplayProcess->state() == QProcess::Running) {
        qDebug() << "CarPlay already running";
        return;
    }

    // Adjust the path to your CarPlay application
    QString carplayAppPath = "/home/testpi5/Documents/CarplayDev/dist/pi-carplay-5.0.0-arm64.AppImage";  // UPDATE THIS PATH

    qDebug() << "Launching CarPlay from:" << carplayAppPath;

    carplayProcess->start(carplayAppPath);

    if (!carplayProcess->waitForStarted(5000)) {
        qDebug() << "Failed to start CarPlay:" << carplayProcess->errorString();
    } else {
        qDebug() << "CarPlay launched successfully";
    }
}

void CarPlayController::stopCarPlay()
{
    if (carplayProcess->state() == QProcess::Running) {
        qDebug() << "Stopping CarPlay...";
        carplayProcess->terminate();

        if (!carplayProcess->waitForFinished(3000)) {
            qDebug() << "CarPlay didn't terminate gracefully, killing...";
            carplayProcess->kill();
        }
    }
}

void CarPlayController::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "CarPlay process finished with exit code:" << exitCode;

    emit carplayExited(exitCode);

    // Check if exit code is 42 (user exited)
    if (exitCode == 42) {
        qDebug() << "CarPlay exited normally (code 42), returning to home";
        emit shouldReturnHome();
    }
}

void CarPlayController::handleProcessError(QProcess::ProcessError error)
{
    qDebug() << "CarPlay process error:" << error;
    qDebug() << "Error string:" << carplayProcess->errorString();
}
