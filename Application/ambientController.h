#ifndef AMBIENTCONTROLLER_H
#define AMBIENTCONTROLLER_H

#include <QObject>
#include <QColor>

/**
 * AmbientController
 *
 * Registered as a QML singleton under the name "ambientController".
 * Wire this up to your actual LED/serial output in the slot implementations.
 *
 * Register in main.cpp:
 *   AmbientController ambientController;
 *   engine.rootContext()->setContextProperty("ambientController", &ambientController);
 */
class AmbientController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int  red        READ red        NOTIFY colourChanged)
    Q_PROPERTY(int  green      READ green      NOTIFY colourChanged)
    Q_PROPERTY(int  blue       READ blue       NOTIFY colourChanged)
    Q_PROPERTY(int  brightness READ brightness NOTIFY brightnessChanged)
    Q_PROPERTY(bool powerOn    READ powerOn    NOTIFY powerChanged)
    Q_PROPERTY(QString mode    READ mode       NOTIFY modeChanged)

public:
    explicit AmbientController(QObject *parent = nullptr);

    int     red()        const { return m_colour.red();   }
    int     green()      const { return m_colour.green(); }
    int     blue()       const { return m_colour.blue();  }
    int     brightness() const { return m_brightness;     }
    bool    powerOn()    const { return m_powerOn;        }
    QString mode()       const { return m_mode;           }

public slots:
    void setColour(int r, int g, int b);
    void setBrightness(int value);
    void setPower(bool on);
    void setMode(const QString &mode);

signals:
    void colourChanged();
    void brightnessChanged();
    void powerChanged();
    void modeChanged();

private:
    void applyToHardware();

    QColor  m_colour     {255, 100, 0};
    int     m_brightness {80};
    bool    m_powerOn    {true};
    QString m_mode       {"Static"};
};

#endif // AMBIENTCONTROLLER_H
