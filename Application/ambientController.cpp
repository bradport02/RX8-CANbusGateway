#include "ambientController.h"
#include <QDebug>

AmbientController::AmbientController(QObject *parent)
    : QObject(parent)
{}

void AmbientController::setColour(int r, int g, int b)
{
    r = qBound(0, r, 255);
    g = qBound(0, g, 255);
    b = qBound(0, b, 255);

    if (m_colour.red() == r && m_colour.green() == g && m_colour.blue() == b)
        return;

    m_colour = QColor(r, g, b);
    emit colourChanged();
    applyToHardware();
}

void AmbientController::setBrightness(int value)
{
    value = qBound(0, value, 100);
    if (m_brightness == value) return;

    m_brightness = value;
    emit brightnessChanged();
    applyToHardware();
}

void AmbientController::setPower(bool on)
{
    if (m_powerOn == on) return;

    m_powerOn = on;
    emit powerChanged();
    applyToHardware();
}

void AmbientController::setMode(const QString &mode)
{
    if (m_mode == mode) return;

    m_mode = mode;
    emit modeChanged();
    applyToHardware();

    // TODO: trigger mode-specific animation timer here if needed
}

void AmbientController::applyToHardware()
{
    if (!m_powerOn) {
        // TODO: send "off" command to your LED/serial interface
        qDebug() << "[Ambient] Lights OFF";
        return;
    }

    // Scale by brightness
    double f  = m_brightness / 100.0;
    int    r  = qRound(m_colour.red()   * f);
    int    g  = qRound(m_colour.green() * f);
    int    b  = qRound(m_colour.blue()  * f);

    qDebug() << "[Ambient]" << m_mode
             << "RGB:" << r << g << b
             << "Brightness:" << m_brightness << "%";

    // ── Replace the block below with your actual hardware call ───────────────
    //
    // Example — serial port write:
    //   QByteArray cmd;
    //   cmd.append((char)r); cmd.append((char)g); cmd.append((char)b);
    //   m_serialPort->write(cmd);
    //
    // Example — CAN frame:
    //   QCanBusFrame frame(0x200, QByteArray::fromRawData("\xRR\xGG\xBB", 3));
    //   m_canDevice->writeFrame(frame);
    // ────────────────────────────────────────────────────────────────────────
}
