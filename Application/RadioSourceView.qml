import QtQuick
import QtQuick.Controls

Rectangle {
    color: "#111118"

    Component.onCompleted: {
        uartController.sendLCDText("Radio")
    }

    property real currentFrequency: 95.5
    property string currentBand: "FM"
    property int currentPreset: -1

    property var presets: [
        {freq: 95.5,  name: "BBC Radio 1"},
        {freq: 98.8,  name: "Heart FM"},
        {freq: 100.0, name: "Kiss FM"},
        {freq: 102.2, name: "Smooth Radio"},
        {freq: 104.9, name: "Capital FM"},
        {freq: 107.8, name: "Radio X"}
    ]

    // ── Band selector (top) ───────────────────────────────────────────────────
    Rectangle {
        id: bandBar
        width: parent.width
        height: 50
        anchors.top: parent.top
        color: "#1a1a22"

        Row {
            anchors.centerIn: parent
            spacing: 20

            Repeater {
                model: ["FM", "AM"]
                delegate: Rectangle {
                    width: 80; height: 35; radius: 8
                    color: currentBand === modelData ? "#1e2035" : "transparent"
                    border.color: currentBand === modelData ? "#4a6fc7" : "#333355"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: currentBand === modelData ? "#7ab3ff" : "#666688"
                        font.pixelSize: 15
                        font.bold: currentBand === modelData
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            currentBand = modelData
                            currentFrequency = modelData === "FM" ? 95.5 : 1000
                        }
                    }
                }
            }
        }
    }

    // ── Preset bar (bottom) ───────────────────────────────────────────────────
    Rectangle {
        id: presetBar
        width: parent.width
        height: 180
        anchors.bottom: parent.bottom
        color: "#0d0d14"

        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Text {
                text: "PRESETS"
                color: "#555d72"
                font.pixelSize: 9
                font.bold: true
                font.letterSpacing: 2.5
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Grid {
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 3
                spacing: 10

                Repeater {
                    model: presets
                    delegate: Rectangle {
                        width: 175; height: 60; radius: 8
                        color: currentPreset === index ? "#1e2035" : "#161622"
                        border.color: currentPreset === index ? "#4a6fc7" : "#2a2a3a"
                        border.width: 1

                        Column {
                            anchors.centerIn: parent
                            spacing: 3
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.name
                                color: currentPreset === index ? "white" : "#aaaacc"
                                font.pixelSize: 12
                                font.bold: currentPreset === index
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.freq.toFixed(1)
                                color: currentPreset === index ? "#7ab3ff" : "#555577"
                                font.pixelSize: 16
                                font.bold: true
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                currentFrequency = modelData.freq
                                currentPreset = index
                                currentBand = "FM"
                            }
                        }
                    }
                }
            }
        }
    }

    // ── Frequency display (fills remaining space between band bar and presets) ─
    Item {
        anchors.top: bandBar.bottom
        anchors.bottom: presetBar.top
        width: parent.width

        // Seek left
        Rectangle {
            width: 60; height: 60; radius: 30
            anchors.left: parent.left
            anchors.leftMargin: 40
            anchors.verticalCenter: parent.verticalCenter
            color: "#1e1e2e"
            border.color: "#333355"; border.width: 1
            Text { anchors.centerIn: parent; text: "◄◄"; color: "white"; font.pixelSize: 22 }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    currentFrequency = currentBand === "FM"
                        ? Math.max(87.5, currentFrequency - 0.1)
                        : Math.max(530,  currentFrequency - 10)
                    currentPreset = -1
                }
            }
        }

        // Seek right
        Rectangle {
            width: 60; height: 60; radius: 30
            anchors.right: parent.right
            anchors.rightMargin: 40
            anchors.verticalCenter: parent.verticalCenter
            color: "#1e1e2e"
            border.color: "#333355"; border.width: 1
            Text { anchors.centerIn: parent; text: "►►"; color: "white"; font.pixelSize: 22 }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    currentFrequency = currentBand === "FM"
                        ? Math.min(108.0, currentFrequency + 0.1)
                        : Math.min(1710,  currentFrequency + 10)
                    currentPreset = -1
                }
            }
        }

        // Centre frequency info
        Column {
            anchors.centerIn: parent
            spacing: 8

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: currentPreset >= 0 ? presets[currentPreset].name : "Manual Tuning"
                color: "#666688"
                font.pixelSize: 16
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: currentBand === "FM"
                    ? currentFrequency.toFixed(1)
                    : Math.round(currentFrequency).toString()
                color: "#7ab3ff"
                font.pixelSize: 80
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: currentBand + (currentBand === "FM" ? " MHz" : " kHz")
                color: "white"
                font.pixelSize: 18
            }

            // Signal bars
            Row {
                spacing: 4
                anchors.horizontalCenter: parent.horizontalCenter
                Repeater {
                    model: 5
                    Rectangle {
                        width: 12
                        height: 16 + (index * 6)
                        color: index < 3 ? "#4a6fc7" : "#2a2a3a"
                        radius: 2
                        anchors.bottom: parent.bottom
                    }
                }
            }
        }
    }
}
