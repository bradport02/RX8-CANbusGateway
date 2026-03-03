import QtQuick
import QtQuick.Controls

Rectangle {
    id: canbusPage
    color: "#1a1a1a"

    // Sample CAN Bus data - replace with real data later
    property var canData: [
        {id: "0x100", name: "Engine RPM", value: "3250", unit: "RPM", timestamp: "12:34:56.123"},
        {id: "0x101", name: "Vehicle Speed", value: "65", unit: "km/h", timestamp: "12:34:56.124"},
        {id: "0x102", name: "Coolant Temp", value: "92", unit: "°C", timestamp: "12:34:56.125"},
        {id: "0x103", name: "Throttle Position", value: "45", unit: "%", timestamp: "12:34:56.126"},
        {id: "0x104", name: "Fuel Level", value: "67", unit: "%", timestamp: "12:34:56.127"},
        {id: "0x105", name: "Battery Voltage", value: "13.8", unit: "V", timestamp: "12:34:56.128"},
        {id: "0x106", name: "Oil Pressure", value: "4.2", unit: "bar", timestamp: "12:34:56.129"},
        {id: "0x107", name: "Air Intake Temp", value: "28", unit: "°C", timestamp: "12:34:56.130"},
        {id: "0x108", name: "Brake Pressure", value: "0", unit: "bar", timestamp: "12:34:56.131"},
        {id: "0x109", name: "Gear Position", value: "4", unit: "", timestamp: "12:34:56.132"}
    ]

    Column {
        anchors.fill: parent
        spacing: 0

        // Header with stats
        Rectangle {
            width: parent.width
            height: 100
            color: "#2a2a2a"

            Column {
                anchors.centerIn: parent
                spacing: 5

                Text {
                    text: "CAN Bus Monitor"
                    color: "white"
                    font.pixelSize: 24
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Row {
                    spacing: 30
                    anchors.horizontalCenter: parent.horizontalCenter

                    Column {
                        spacing: 2
                        Text {
                            text: "Status"
                            color: "#888888"
                            font.pixelSize: 12
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Rectangle {
                            width: 60
                            height: 25
                            color: canController.isConnected ? "#00ff00" : "#ff0000"
                            radius: 12
                            anchors.horizontalCenter: parent.horizontalCenter
                            Text {
                                text: canController.isConnected ? "Active" : "Offline"
                                color: "#000000"
                                font.pixelSize: 12
                                font.bold: true
                                anchors.centerIn: parent
                            }
                        }
                    }

                    Column {
                        spacing: 2
                        Text {
                            text: "Messages"
                            color: "#888888"
                            font.pixelSize: 12
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: canController.messageCount.toString()
                            color: "#4ecdc4"
                            font.pixelSize: 16
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    Column {
                        spacing: 2
                        Text {
                            text: "Bus Load"
                            color: "#888888"
                            font.pixelSize: 12
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: canController.busLoad + "%"
                            color: "#4ecdc4"
                            font.pixelSize: 16
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }

        // Control buttons
        Rectangle {
            width: parent.width
            height: 60
            color: "#1a1a1a"
            border.color: "#3a3a3a"
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 15

                Button {
                    text: canController.isConnected ? "Disconnect" : "Connect CAN"
                    width: 140
                    height: 40

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: canController.isConnected ? "#ff4444" : "#00ff00"
                        radius: 5
                    }

                    onClicked: {
                        if (!canController.isConnected) {
                            canController.connectToCAN("can0")
                        } else {
                            canController.disconnectCAN()
                        }
                    }
                }

                Button {
                    text: "View Raw Data"
                    width: 140
                    height: 40

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: "#4ecdc4"
                        radius: 5
                    }

                    onClicked: {
                        stackView.push(canRawDataPage)
                    }
                }
            }
        }

        // CAN Data List
        Rectangle {
            width: parent.width
            height: parent.height - 160
            color: "#0a0a0a"

            ListView {
                id: canListView
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5
                clip: true

                model: canData

                delegate: Rectangle {
                    width: canListView.width
                    height: 70
                    color: "#1a1a1a"
                    radius: 5
                    border.color: "#3a3a3a"
                    border.width: 1

                    Row {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        // CAN ID
                        Rectangle {
                            width: 80
                            height: parent.height
                            color: "#2a2a2a"
                            radius: 3

                            Text {
                                text: modelData.id
                                color: "#4ecdc4"
                                font.pixelSize: 14
                                font.family: "Courier"
                                anchors.centerIn: parent
                            }
                        }

                        // Name and Value
                        Column {
                            width: parent.width - 280
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 5

                            Text {
                                text: modelData.name
                                color: "white"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            Text {
                                text: "Last: " + modelData.timestamp
                                color: "#666666"
                                font.pixelSize: 11
                                font.family: "Courier"
                            }
                        }

                        // Current Value
                        Rectangle {
                            width: 120
                            height: parent.height
                            color: "#2a2a2a"
                            radius: 3
                            anchors.verticalCenter: parent.verticalCenter

                            Column {
                                anchors.centerIn: parent
                                spacing: 2

                                Text {
                                    text: modelData.value
                                    color: "#00ff00"
                                    font.pixelSize: 22
                                    font.bold: true
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                Text {
                                    text: modelData.unit
                                    color: "#888888"
                                    font.pixelSize: 12
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }

                        // Trend indicator
                        Text {
                            text: "▲"
                            color: "#00ff00"
                            font.pixelSize: 20
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
            }
        }
    }
}
