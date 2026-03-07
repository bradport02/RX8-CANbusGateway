import QtQuick
import QtQuick.Controls
import Qt.labs.settings 1.0

Rectangle {
    id: climateControlPage
    color: "#1a1a1a"

    // ── Persistent settings ───────────────────────────────────────────────────
    Settings {
        id: settings
        category: "climate"
        property real temperature:  22.0
        property int  fanSpeed:     3
        property bool acEnabled:    false
        property bool demistEnabled: false
        property string ventMode:   "face"
        property bool recirculation: false
        property bool autoMode:     false
    }

    // ── Climate state — initialised from saved settings ───────────────────────
    property real   temperature:   settings.temperature
    property int    fanSpeed:      settings.fanSpeed
    property bool   acEnabled:     settings.acEnabled
    property bool   demistEnabled: settings.demistEnabled
    property string ventMode:      settings.ventMode
    property bool   recirculation: settings.recirculation
    property bool   autoMode:      settings.autoMode

    // Persist on every change
    onTemperatureChanged:   settings.temperature   = temperature
    onFanSpeedChanged:      settings.fanSpeed      = fanSpeed
    onAcEnabledChanged:     settings.acEnabled     = acEnabled
    onDemistEnabledChanged: settings.demistEnabled = demistEnabled
    onVentModeChanged:      settings.ventMode      = ventMode
    onRecirculationChanged: settings.recirculation = recirculation
    onAutoModeChanged:      settings.autoMode      = autoMode

    // ── Helpers ───────────────────────────────────────────────────────────────
    function tempStr(t) {
        return (t % 1 === 0) ? t.toFixed(0) : t.toFixed(1)
    }
    function ventModeInt() {
        return ventMode === "face" ? 0 : ventMode === "feet" ? 1 : ventMode === "both" ? 2 : 3
    }

    Column {
        anchors.fill: parent
        spacing: 0

        // ── Header ────────────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 80
            color: "#2a2a2a"

            Text {
                text: "Climate Control"
                color: "white"
                font.pixelSize: 28
                font.bold: true
                anchors.centerIn: parent
            }
        }

        // ── Main control area ─────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: parent.height - 80
            color: "#1a1a1a"

            Row {
                anchors.centerIn: parent
                spacing: 80

                // ── Temperature Control ───────────────────────────────────────
                Column {
                    spacing: 20

                    Text {
                        text: "Temperature"
                        color: "#888888"
                        font.pixelSize: 18
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Column {
                        spacing: 15
                        anchors.horizontalCenter: parent.horizontalCenter

                        Rectangle {
                            width: 60; height: 60; radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: tempUpArea.pressed ? "#4a4a4a" : "#3a3a3a"
                            Text { text: "▲"; color: "white"; font.pixelSize: 40; anchors.centerIn: parent }
                            MouseArea {
                                id: tempUpArea
                                anchors.fill: parent
                                onClicked: {
                                    if (temperature < 32.0) {
                                        temperature = Math.round((temperature + 0.5) * 10) / 10
                                        uartController.sendTemperature(0x03, temperature)
                                    }
                                }
                            }
                        }

                        Rectangle {
                            width: 130; height: 130; radius: 65
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: "#2a2a2a"
                            border.color: "#4ecdc4"; border.width: 3

                            Column {
                                anchors.centerIn: parent
                                spacing: 0
                                Text {
                                    text: tempStr(temperature)
                                    color: "#4ecdc4"
                                    font.pixelSize: temperature >= 10 ? 48 : 56
                                    font.bold: true
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Text {
                                    text: "°C"
                                    color: "white"
                                    font.pixelSize: 22
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }

                        Rectangle {
                            width: 60; height: 60; radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: tempDownArea.pressed ? "#4a4a4a" : "#3a3a3a"
                            Text { text: "▼"; color: "white"; font.pixelSize: 40; anchors.centerIn: parent }
                            MouseArea {
                                id: tempDownArea
                                anchors.fill: parent
                                onClicked: {
                                    if (temperature > 15.0) {
                                        temperature = Math.round((temperature - 0.5) * 10) / 10
                                        uartController.sendTemperature(0x04, temperature)
                                    }
                                }
                            }
                        }
                    }
                }

                // ── Fan Speed Control ─────────────────────────────────────────
                Column {
                    spacing: 20

                    Text {
                        text: "Fan Speed"
                        color: "#888888"
                        font.pixelSize: 18
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Column {
                        spacing: 15
                        anchors.horizontalCenter: parent.horizontalCenter

                        Rectangle {
                            width: 60; height: 60; radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: fanUpArea.pressed ? "#4a4a4a" : "#3a3a3a"
                            Text { text: "▲"; color: "white"; font.pixelSize: 40; anchors.centerIn: parent }
                            MouseArea {
                                id: fanUpArea
                                anchors.fill: parent
                                onClicked: {
                                    if (fanSpeed < 7) {
                                        fanSpeed++
                                        uartController.send(0x05, fanSpeed)
                                    }
                                }
                            }
                        }

                        Rectangle {
                            width: 130; height: 130; radius: 65
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: "#2a2a2a"
                            border.color: fanSpeed === 0 ? "#555555" : "#4ecdc4"
                            border.width: 3

                            Column {
                                anchors.centerIn: parent
                                spacing: 4
                                Text {
                                    text: fanSpeed === 0 ? "OFF" : fanSpeed.toString()
                                    color: fanSpeed === 0 ? "#666666" : "#4ecdc4"
                                    font.pixelSize: fanSpeed === 0 ? 32 : 56
                                    font.bold: true
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Text {
                                    text: "FAN"
                                    color: fanSpeed === 0 ? "#444444" : "#ffffff"
                                    font.pixelSize: 20
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }

                        Rectangle {
                            width: 60; height: 60; radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: fanDownArea.pressed ? "#4a4a4a" : "#3a3a3a"
                            Text { text: "▼"; color: "white"; font.pixelSize: 40; anchors.centerIn: parent }
                            MouseArea {
                                id: fanDownArea
                                anchors.fill: parent
                                onClicked: {
                                    if (fanSpeed > 0) {
                                        fanSpeed--
                                        if (fanSpeed === 0)
                                            uartController.send(0x0C, 0)
                                        else
                                            uartController.send(0x06, fanSpeed)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ── Control buttons ───────────────────────────────────────────────
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 40
                spacing: 20

                // AC
                Button {
                    width: 100; height: 60
                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 4
                        Text { text: "❄️"; font.pixelSize: 22; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "A/C"; color: acEnabled ? "#000000" : "white"; font.pixelSize: 14; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                    background: Rectangle { color: acEnabled ? "#4ecdc4" : "#3a3a3a"; radius: 10; border.color: acEnabled ? "#4ecdc4" : "#555555"; border.width: 2 }
                    onClicked: {
                        acEnabled = !acEnabled
                        uartController.send(0x08, acEnabled ? 1 : 0)
                    }
                }

                // Demist
                Button {
                    width: 100; height: 60
                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 4
                        Text { text: "🌬️"; font.pixelSize: 22; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "Demist"; color: demistEnabled ? "#000000" : "white"; font.pixelSize: 14; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                    background: Rectangle { color: demistEnabled ? "#ff9900" : "#3a3a3a"; radius: 10; border.color: demistEnabled ? "#ff9900" : "#555555"; border.width: 2 }
                    onClicked: {
                        demistEnabled = !demistEnabled
                        uartController.send(0x07, demistEnabled ? 1 : 0)
                    }
                }

                // Vent mode
                Button {
                    width: 100; height: 60
                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 4
                        Text {
                            text: ventMode === "face" ? "😊" : ventMode === "feet" ? "🦶" : ventMode === "both" ? "↕️" : "De"
                            font.pixelSize: 22
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: ventMode === "face" ? "Face" : ventMode === "feet" ? "Feet" : ventMode === "both" ? "Both" : "Demist" //ventMode === "demist" ? "Demist"
                            color: "white"; font.pixelSize: 14; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    background: Rectangle { color: "#3a3a3a"; radius: 10; border.color: "#4ecdc4"; border.width: 2 }
                    onClicked: {
                        if (ventMode === "face")      ventMode = "feet"
                        else if (ventMode === "feet") ventMode = "both"
                        else if (ventMode === "both") ventMode = "demist"
                        else                          ventMode = "face"
                        uartController.send(0x09, ventModeInt())
                    }
                }

                // Recirculation
                Button {
                    width: 100; height: 60
                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 4
                        Text { text: "🔄"; font.pixelSize: 22; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "Recirc"; color: recirculation ? "#000000" : "white"; font.pixelSize: 14; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                    background: Rectangle { color: recirculation ? "#4ecdc4" : "#3a3a3a"; radius: 10; border.color: recirculation ? "#4ecdc4" : "#555555"; border.width: 2 }
                    onClicked: {
                        recirculation = !recirculation
                        uartController.send(0x0A, recirculation ? 1 : 0)
                    }
                }

                // Auto
                Button {
                    width: 100; height: 60
                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 4
                        Text { text: "Auto"; font.pixelSize: 22; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "Auto"; color: autoMode ? "#000000" : "white"; font.pixelSize: 14; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                    background: Rectangle { color: autoMode ? "#4ecdc4" : "#3a3a3a"; radius: 10; border.color: autoMode ? "#4ecdc4" : "#555555"; border.width: 2 }
                    onClicked: {
                        autoMode = !autoMode
                        uartController.send(0x0B, autoMode ? 1 : 0)
                    }
                }
            }
        }
    }
}
