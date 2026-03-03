import QtQuick
import QtQuick.Controls

Rectangle {
    id: radioPage
    color: "#1a1a1a"

    // Radio state
    property real currentFrequency: 95.5
    property string currentBand: "FM"
    property int currentPreset: -1

    // Preset stations (frequency, name)
    property var presets: [
        {freq: 95.5, name: "BBC Radio 1"},
        {freq: 98.8, name: "Heart FM"},
        {freq: 100.0, name: "Kiss FM"},
        {freq: 102.2, name: "Smooth Radio"},
        {freq: 104.9, name: "Capital FM"},
        {freq: 107.8, name: "Radio X"}
    ]

    Column {
        anchors.fill: parent
        spacing: 0

        // Band selector at top
        Rectangle {
            width: parent.width
            height: 50
            color: "#2a2a2a"

            Row {
                anchors.centerIn: parent
                spacing: 20

                Button {
                    text: "FM"
                    width: 80
                    height: 35

                    contentItem: Text {
                        text: parent.text
                        color: currentBand === "FM" ? "#4ecdc4" : "white"
                        font.pixelSize: 16
                        font.bold: currentBand === "FM"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: currentBand === "FM" ? "#3a3a3a" : "#2a2a2a"
                        radius: 5
                        border.color: currentBand === "FM" ? "#4ecdc4" : "#4a4a4a"
                        border.width: 2
                    }

                    onClicked: {
                        currentBand = "FM"
                        currentFrequency = 95.5
                    }
                }

                Button {
                    text: "AM"
                    width: 80
                    height: 35

                    contentItem: Text {
                        text: parent.text
                        color: currentBand === "AM" ? "#4ecdc4" : "white"
                        font.pixelSize: 16
                        font.bold: currentBand === "AM"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: currentBand === "AM" ? "#3a3a3a" : "#2a2a2a"
                        radius: 5
                        border.color: currentBand === "AM" ? "#4ecdc4" : "#4a4a4a"
                        border.width: 2
                    }

                    onClicked: {
                        currentBand = "AM"
                        currentFrequency = 1000
                    }
                }
            }
        }

        // Main frequency display area
        Rectangle {
            width: parent.width
            height: parent.height - 50 - 200  // Remaining space
            color: "#1a1a1a"

            Column {
                anchors.centerIn: parent
                spacing: 10

                // Station name (if preset)
                Text {
                    text: currentPreset >= 0 ? presets[currentPreset].name : "Manual Tuning"
                    color: "#888888"
                    font.pixelSize: 18
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Huge frequency display
                Text {
                    text: currentBand === "FM" ?
                          currentFrequency.toFixed(1) :
                          Math.round(currentFrequency).toString()
                    color: "#4ecdc4"
                    font.pixelSize: 96
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Band label
                Text {
                    text: currentBand + " MHz"
                    color: "white"
                    font.pixelSize: 20
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Signal strength indicator
                Row {
                    spacing: 5
                    anchors.horizontalCenter: parent.horizontalCenter

                    Repeater {
                        model: 5
                        Rectangle {
                            width: 15
                            height: 30 + (index * 8)
                            color: index < 3 ? "#4ecdc4" : "#3a3a3a"
                            radius: 3
                            anchors.bottom: parent.bottom
                        }
                    }
                }
            }

            // Seek buttons (left/right of center)
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 40
                spacing: 200

                // Seek Down
                Rectangle {
                    width: 70
                    height: 70
                    color: "#3a3a3a"
                    radius: 35

                    Text {
                        text: "◄◄"
                        color: "white"
                        font.pixelSize: 28
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (currentBand === "FM") {
                                currentFrequency = Math.max(87.5, currentFrequency - 0.1)
                            } else {
                                currentFrequency = Math.max(530, currentFrequency - 10)
                            }
                            currentPreset = -1
                            // TODO: Send tuner command
                        }
                    }
                }

                // Seek Up
                Rectangle {
                    width: 70
                    height: 70
                    color: "#3a3a3a"
                    radius: 35

                    Text {
                        text: "►►"
                        color: "white"
                        font.pixelSize: 28
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (currentBand === "FM") {
                                currentFrequency = Math.min(108.0, currentFrequency + 0.1)
                            } else {
                                currentFrequency = Math.min(1710, currentFrequency + 10)
                            }
                            currentPreset = -1
                            // TODO: Send tuner command
                        }
                    }
                }
            }
        }

        // Preset buttons at bottom
        Rectangle {
            width: parent.width
            height: 200
            color: "#0a0a0a"

            Column {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    text: "Presets"
                    color: "#888888"
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Two rows of 3 presets
                Grid {
                    anchors.horizontalCenter: parent.horizontalCenter
                    columns: 3
                    spacing: 15

                    Repeater {
                        model: presets

                        Rectangle {
                            width: 190
                            height: 70
                            color: currentPreset === index ? "#4a4a4a" : "#2a2a2a"
                            radius: 8
                            border.color: currentPreset === index ? "#4ecdc4" : "#3a3a3a"
                            border.width: 2

                            Column {
                                anchors.centerIn: parent
                                spacing: 5

                                Text {
                                    text: modelData.name
                                    color: "white"
                                    font.pixelSize: 14
                                    font.bold: currentPreset === index
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    elide: Text.ElideRight
                                    width: 170
                                    horizontalAlignment: Text.AlignHCenter
                                }

                                Text {
                                    text: modelData.freq.toFixed(1)
                                    color: "#4ecdc4"
                                    font.pixelSize: 18
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    currentFrequency = modelData.freq
                                    currentPreset = index
                                    currentBand = "FM"
                                    // TODO: Send tuner command
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Swipe gestures for quick station changes
    // Swipe gestures for quick station changes
    MouseArea {
        anchors.fill: parent
        anchors.topMargin: 50  // Below band selector
        anchors.bottomMargin: 200  // Don't interfere with presets
        propagateComposedEvents: true  // Let other MouseAreas work

        property real startX: 0

        onPressed: function(mouse) {
            startX = mouse.x
            mouse.accepted = false  // Allow buttons underneath to work
        }

        onReleased: function(mouse) {
            var deltaX = mouse.x - startX

            if (Math.abs(deltaX) > 100) {  // Swipe threshold
                if (deltaX > 0) {
                    // Swipe right - next station
                    if (currentBand === "FM") {
                        currentFrequency = Math.min(108.0, currentFrequency + 0.1)
                    } else {
                        currentFrequency = Math.min(1710, currentFrequency + 10)
                    }
                } else {
                    // Swipe left - previous station
                    if (currentBand === "FM") {
                        currentFrequency = Math.max(87.5, currentFrequency - 0.1)
                    } else {
                        currentFrequency = Math.max(530, currentFrequency - 10)
                    }
                }
                currentPreset = -1
            }
            mouse.accepted = false
        }
    }
}
