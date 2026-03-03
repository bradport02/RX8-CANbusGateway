import QtQuick
import QtQuick.Controls

Rectangle {
    id: climateControlPage
    color: "#1a1a1a"

    // Climate control state
    property int temperature: 22  // °C
    property int fanSpeed: 3      // 1-7
    property bool acEnabled: false
    property bool demistEnabled: false
    property string ventMode: "face"  // face, feet, both
    property bool recirculation: false

    Column {
        anchors.fill: parent
        spacing: 0

        // Header
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

        // Main control area
        Rectangle {
            width: parent.width
            height: parent.height - 80
            color: "#1a1a1a"

            Row {
                anchors.centerIn: parent
                spacing: 80

                // Temperature Control (Left Side)
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

                        // Up arrow
                        Rectangle {
                            width: 60
                            height: 60
                            color: "#3a3a3a"
                            radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter

                            Text {
                                text: "▲"
                                color: "white"
                                font.pixelSize: 40
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (temperature < 32) {
                                        temperature++
                                        // Send UART command
                                        uartController.sendPacket(0x10, temperature.toString())
                                    }
                                }
                            }
                        }

                        // Temperature display
                        Rectangle {
                            width: 120
                            height: 120
                            color: "#2a2a2a"
                            radius: 60
                            border.color: "#4ecdc4"
                            border.width: 3
                            anchors.horizontalCenter: parent.horizontalCenter

                            Column {
                                anchors.centerIn: parent
                                spacing: 0

                                Text {
                                    text: temperature.toString()
                                    color: "#4ecdc4"
                                    font.pixelSize: 56
                                    font.bold: true
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                Text {
                                    text: "°C"
                                    color: "white"
                                    font.pixelSize: 24
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }

                        // Down arrow
                        Rectangle {
                            width: 60
                            height: 60
                            color: "#3a3a3a"
                            radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter

                            Text {
                                text: "▼"
                                color: "white"
                                font.pixelSize: 40
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (temperature > 18) {
                                        temperature--
                                        // Send UART command
                                        uartController.sendPacket(0x10, temperature.toString())
                                    }
                                }
                            }
                        }
                    }
                }

                // Fan Speed Control (Right Side)
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

                        // Up arrow
                        Rectangle {
                            width: 60
                            height: 60
                            color: "#3a3a3a"
                            radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter

                            Text {
                                text: "▲"
                                color: "white"
                                font.pixelSize: 40
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (fanSpeed < 7) {
                                        fanSpeed++
                                        // Send UART command
                                        uartController.sendPacket(0x11, fanSpeed.toString())
                                    }
                                }
                            }
                        }

                        // Fan speed display
                        Rectangle {
                            width: 120
                            height: 120
                            color: "#2a2a2a"
                            radius: 60
                            border.color: "#4ecdc4"
                            border.width: 3
                            anchors.horizontalCenter: parent.horizontalCenter

                            Column {
                                anchors.centerIn: parent
                                spacing: 5

                                Text {
                                    text: fanSpeed.toString()
                                    color: "#4ecdc4"
                                    font.pixelSize: 56
                                    font.bold: true
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                // Fan icon
                                Text {
                                    color: "#ffffff"
                                    text: "FAN"
                                    font.pixelSize: 24
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }

                        // Down arrow
                        Rectangle {
                            width: 60
                            height: 60
                            color: "#3a3a3a"
                            radius: 10
                            anchors.horizontalCenter: parent.horizontalCenter

                            Text {
                                text: "▼"
                                color: "white"
                                font.pixelSize: 40
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (fanSpeed > 1) {
                                        fanSpeed--
                                        // Send UART command
                                        uartController.sendPacket(0x11, fanSpeed.toString())
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Control buttons at bottom
            Row {
                y: 150
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 150
                layoutDirection: Qt.LeftToRight
                anchors.horizontalCenterOffset: -244
                rotation: 90
                spacing: 20

                // AC Button
                Button {
                    width: 50
                    height: 100

                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 5

                        Text {
                            text: "❄️"
                            rotation: -90
                            font.pixelSize: 32
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "A/C"
                            rotation: -90
                            color: acEnabled ? "#000000" : "white"
                            font.pixelSize: 18
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    background: Rectangle {
                        color: acEnabled ? "#4ecdc4" : "#3a3a3a"
                        radius: 10
                        border.color: acEnabled ? "#4ecdc4" : "#555555"
                        border.width: 2
                    }

                    onClicked: {
                        acEnabled = !acEnabled
                        uartController.sendPacket(0x12, acEnabled ? "1" : "0")
                    }
                }

                // Demist Button
                Button {
                    width: 50
                    height: 100

                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 5

                        Text {
                            text: "  "
                            rotation: -90
                            font.pixelSize: 32
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "Demist"
                            rotation: -90
                            color: demistEnabled ? "#000000" : "white"
                            font.pixelSize: 18
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    background: Rectangle {
                        color: demistEnabled ? "#ff9900" : "#3a3a3a"
                        radius: 10
                        border.color: demistEnabled ? "#ff9900" : "#555555"
                        border.width: 2
                    }

                    onClicked: {
                        demistEnabled = !demistEnabled
                        uartController.sendPacket(0x13, demistEnabled ? "1" : "0")
                    }
                }

                // Vent Mode Button
                Button {
                    width: 50
                    height: 100

                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 5

                        Text {
                            text: {
                                if (ventMode === "face") return "  "
                                if (ventMode === "feet") return "  "
                                return "  "
                            }
                            font.pixelSize: 32
                            rotation: -90
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: {
                                if (ventMode === "face") return "Face"
                                if (ventMode === "feet") return "Feet"
                                return "Both"
                            }
                            color: "white"
                            font.pixelSize: 18
                            font.bold: true
                            rotation: -90
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    background: Rectangle {
                        color: "#3a3a3a"
                        radius: 10
                        border.color: "#4ecdc4"
                        border.width: 2
                    }

                    onClicked: {
                        // Cycle through modes
                        if (ventMode === "face") {
                            ventMode = "feet"
                        } else if (ventMode === "feet") {
                            ventMode = "both"
                        } else {
                            ventMode = "face"
                        }
                        uartController.sendPacket(0x14, ventMode)
                    }
                }

                // Recirculation Button
                Button {
                    width: 50
                    height: 100

                    contentItem: Column {
                        anchors.centerIn: parent
                        spacing: 5

                        Text {
                            text: "  "
                            font.pixelSize: 32
                            rotation: -90
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "Recirc"
                            color: recirculation ? "#000000" : "white"
                            font.pixelSize: 18
                            rotation: -90
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    background: Rectangle {
                        color: recirculation ? "#4ecdc4" : "#3a3a3a"
                        radius: 10
                        border.color: recirculation ? "#4ecdc4" : "#555555"
                        border.width: 2
                    }

                    onClicked: {
                        recirculation = !recirculation
                        uartController.sendPacket(0x15, recirculation ? "1" : "0")
                    }
                }
            }
        }
    }
}
