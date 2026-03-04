import QtQuick
import QtQuick.Controls

Rectangle {
    id: climateControlPage
    color: "#1a1a1a"

    // Climate control state
    property real temperature: 22.0  // 15.0–32.0 in 0.5 steps
    property int fanSpeed: 3          // 0=off, 1–7
    property bool acEnabled: false
    property bool demistEnabled: false
    property string ventMode: "face"  // face, feet, both
    property bool recirculation: false

    // Format temperature: show ".0" as whole number, ".5" as decimal
    function tempStr(t) {
        return (t % 1 === 0) ? t.toFixed(0) : t.toFixed(1)
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

                        // Up
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

                        // Temperature display
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

                        // Down
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

                        // Up
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
                                        uartController.sendPacket(0x11, fanSpeed.toString())
                                    }
                                }
                            }
                        }

                        // Fan display
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

                        // Down
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
                                        uartController.sendPacket(0x11, fanSpeed.toString())
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ── Control buttons at bottom ─────────────────────────────────────
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
                    onClicked: { acEnabled = !acEnabled; uartController.sendPacket(0x12, acEnabled ? "1" : "0") }
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
                    onClicked: { demistEnabled = !demistEnabled; uartController.sendPacket(0x13, demistEnabled ? "1" : "0") }
                }

                // Vent mode
                Button {
                    width: 100; height: 60
                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 4
                        Text {
                            text: ventMode === "face" ? "😊" : ventMode === "feet" ? "🦶" : "↕️"
                            font.pixelSize: 22
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: ventMode === "face" ? "Face" : ventMode === "feet" ? "Feet" : "Both"
                            color: "white"; font.pixelSize: 14; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    background: Rectangle { color: "#3a3a3a"; radius: 10; border.color: "#4ecdc4"; border.width: 2 }
                    onClicked: {
                        if (ventMode === "face") ventMode = "feet"
                        else if (ventMode === "feet") ventMode = "both"
                        else ventMode = "face"
                        uartController.sendPacket(0x14, ventMode)
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
                    onClicked: { recirculation = !recirculation; uartController.sendPacket(0x15, recirculation ? "1" : "0") }
                }
            }
        }
    }
}
