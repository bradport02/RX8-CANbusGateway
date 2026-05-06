import QtQuick
import QtQuick.Controls

Rectangle {
    color: "#1a1a1a"

    Component.onCompleted: {
        uartController.sendLCDText("Settings")
    }

    Flickable {
        anchors.fill: parent
        contentHeight: settingsColumn.height
        clip: true

        Column {
            id: settingsColumn
            width: parent.width
            spacing: 0

            // Section Header
            Rectangle {
                width: parent.width
                height: 60
                color: "#2a2a2a"

                Text {
                    text: "Display Settings"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Brightness Setting
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Text {
                        text: "Brightness"
                        color: "white"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                        width: 150
                    }

                    Slider {
                        width: parent.width - 170
                        anchors.verticalCenter: parent.verticalCenter
                        from: 0
                        to: 100
                        value: 80
                    }
                }
            }

            // Toggle Switch Example
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Text {
                        text: "Dark Mode"
                        color: "white"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 100
                    }

                    Switch {
                        anchors.verticalCenter: parent.verticalCenter
                        checked: true
                    }
                }
            }

            // Ambient Lighting
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5
                        width: parent.width - 80

                        Text {
                            text: "Ambient Lighting"
                            color: "white"
                            font.pixelSize: 18
                        }

                        Text {
                            text: "View and modify the ambient lighting colour"
                            color: "#888888"
                            font.pixelSize: 14
                        }
                    }

                    Text {
                        text: "›"
                        color: "#888888"
                        font.pixelSize: 32
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stackView.push(lightingPage)
                }
            }

            // Ambient Lighting
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5
                        width: parent.width - 80

                        Text {
                            text: "Date and Time"
                            color: "white"
                            font.pixelSize: 18
                        }

                        Text {
                            text: "Adjust the system date and time."
                            color: "#888888"
                            font.pixelSize: 14
                        }
                    }

                    Text {
                        text: "›"
                        color: "#888888"
                        font.pixelSize: 32
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stackView.push(clockSettingsPage)
                }
            }
            // Another Section
            Rectangle {
                width: parent.width
                height: 60
                color: "#2a2a2a"

                Text {
                    text: "Audio Settings"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Text {
                        text: "Beep Tone"
                        color: "white"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 100
                    }

                    Switch {
                        anchors.verticalCenter: parent.verticalCenter
                        checked: true
                    }
                }
            }

            // Tone Settings
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5
                        width: parent.width - 80

                        Text {
                            text: "Tone Settings"
                            color: "white"
                            font.pixelSize: 18
                        }

                        Text {
                            text: "Adjust Bass, Mids, Trebble, Fader and Balance."
                            color: "#888888"
                            font.pixelSize: 14
                        }
                    }

                    Text {
                        text: "›"
                        color: "#888888"
                        font.pixelSize: 32
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stackView.push(audioSettingsPage)
                }


            }

            // Bluetooth Connection
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5
                        width: parent.width - 80

                        Text {
                            text: "Bluetooth Settings"
                            color: "white"
                            font.pixelSize: 18
                        }

                        Text {
                            text: "Pair and Connect Bluetooth Devices"
                            color: "#888888"
                            font.pixelSize: 14
                        }
                    }

                    Text {
                        text: "›"
                        color: "#888888"
                        font.pixelSize: 32
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stackView.push(bluetoothSettingsPage)
                }
            }

            // Dropdown/ComboBox Example
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Text {
                        text: "Audio Source"
                        color: "white"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                        width: 150
                    }

                    ComboBox {
                        width: 200
                        anchors.verticalCenter: parent.verticalCenter
                        model: ["Radio", "Bluetooth", "AUX", "Carplay"]
                    }
                }
            }

            // Diagnostics Section
            Rectangle {
                width: parent.width
                height: 60
                color: "#2a2a2a"

                Text {
                    text: "Diagnostics"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // CAN Bus Button
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5
                        width: parent.width - 80

                        Text {
                            text: "CAN Bus Monitor"
                            color: "white"
                            font.pixelSize: 18
                        }

                        Text {
                            text: "View vehicle diagnostics and data"
                            color: "#888888"
                            font.pixelSize: 14
                        }
                    }

                    Text {
                        text: "›"
                        color: "#888888"
                        font.pixelSize: 32
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stackView.push(canbusPage)
                }
            }

            // System Section
            Rectangle {
                width: parent.width
                height: 60
                color: "#2a2a2a"

                Text {
                    text: "System"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Button Example
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#3a3a3a"
                border.width: 1

                Button {
                    text: "Reset to Defaults"
                    anchors.centerIn: parent
                    width: 200
                    height: 50

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: "#ff4444"
                        radius: 5
                    }

                    onClicked: {
                        // Reset logic here
                    }
                }
            }

            // Button Example
            Rectangle {
                width: parent.width
                height: 80
                color: "#1a1a1a"
                border.color: "#1a1a1a"
                border.width: 1

                Button {
                    text: "Close Application"
                    anchors.centerIn: parent
                    width: 200
                    height: 50

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: "#3a3a3a"
                        radius: 5
                    }

                    onClicked: {
                         Qt.quit()
                    }
                }
            }

            // Add spacing at bottom for easier scrolling
            Item {
                width: parent.width
                height: 40
            }
        }
    }
}
