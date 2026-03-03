import QtQuick
import QtQuick.Controls

Rectangle {
    id: canRawDataPage
    color: "#1a1a1a"

    // Snapshot of messages when page opens
    property var snapshotMessages: []
    property int snapshotCount: 0
    property bool wasMonitoring: false
    property bool autoRefresh: true  // Auto-refresh toggle

    Component.onCompleted: {
        // Check if monitoring was active
        wasMonitoring = !canController.isPaused

        // Pause to take snapshot
        if (!canController.isPaused) {
            canController.togglePause()
        }

        // Copy messages to local snapshot
        refreshSnapshot()

        // Resume monitoring after snapshot is taken
        if (wasMonitoring) {
            resumeTimer.start()
        }

        // Start auto-refresh timer
        if (autoRefresh) {
            autoRefreshTimer.start()
        }
    }

    Component.onDestruction: {
        autoRefreshTimer.stop()

        // Make sure monitoring state is correct when leaving
        if (wasMonitoring && canController.isPaused) {
            canController.togglePause()
        }
    }

    function refreshSnapshot() {
        snapshotMessages = canController.rawMessages.slice(Math.max(0, canController.rawMessages.length - 200))
        snapshotCount = canController.rawMessageCount

        // Scroll to bottom after refresh
        rawDataListView.positionViewAtEnd()
    }

    // Timer to resume after initial snapshot
    Timer {
        id: resumeTimer
        interval: 100
        onTriggered: {
            if (canController.isPaused) {
                canController.togglePause()
            }
        }
    }

    // Auto-refresh timer - updates every second
    Timer {
        id: autoRefreshTimer
        interval: 100  // Refresh every 1 second
        running: false
        repeat: true
        onTriggered: {
            if (autoRefresh && !canController.isPaused) {
                refreshSnapshot()
            }
        }
    }

    Column {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            width: parent.width
            height: 80
            color: "#2a2a2a"

            Column {
                anchors.centerIn: parent
                spacing: 5

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        text: "CAN Bus Raw Data"
                        color: "white"
                        font.pixelSize: 24
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // Live indicator
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 10
                        color: !canController.isPaused ? "#00ff00" : "#ff0000"
                        anchors.verticalCenter: parent.verticalCenter

                        SequentialAnimation on opacity {
                            running: !canController.isPaused && autoRefresh
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 500 }
                            NumberAnimation { to: 1.0; duration: 500 }
                        }
                    }

                    Text {
                        text: {
                            if (canController.isPaused) return "Paused"
                            if (autoRefresh) return "Live (1s refresh)"
                            return "Recording"
                        }
                        color: !canController.isPaused ? "#00ff00" : "#ff0000"
                        font.pixelSize: 14
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    text: "Total Messages: " + canController.messageCount + " (Snapshot: " + snapshotMessages.length + ")"
                    color: "#4ecdc4"
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
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
                spacing: 10

                Button {
                    text: autoRefresh ? "Auto: ON" : "Auto: OFF"
                    width: 100
                    height: 40

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: autoRefresh ? "#00ff00" : "#666666"
                        radius: 5
                    }

                    onClicked: {
                        autoRefresh = !autoRefresh
                        if (autoRefresh) {
                            autoRefreshTimer.start()
                        } else {
                            autoRefreshTimer.stop()
                        }
                    }
                }

                Button {
                    text: "Refresh Now"
                    width: 110
                    height: 40

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: "#4ecdc4"
                        radius: 5
                    }

                    onClicked: {
                        refreshSnapshot()
                    }
                }

                Button {
                    text: canController.isPaused ? "Resume" : "Pause"
                    width: 90
                    height: 40

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: canController.isPaused ? "#00ff00" : "#ff9900"
                        radius: 5
                    }

                    onClicked: {
                        canController.togglePause()
                        if (!canController.isPaused && autoRefresh) {
                            autoRefreshTimer.start()
                        } else {
                            autoRefreshTimer.stop()
                        }
                    }
                }

                Button {
                    text: "Clear"
                    width: 80
                    height: 40

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: "#ff4444"
                        radius: 5
                    }

                    onClicked: {
                        canController.clearMessages()
                        snapshotMessages = []
                        snapshotCount = 0
                    }
                }

                Button {
                    text: "Export"
                    width: 80
                    height: 40

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: "#4ecdc4"
                        radius: 5
                    }

                    onClicked: {
                        canController.exportToCSV("/home/testpi5/canbus_log.csv")
                    }
                }
            }
        }

        // Column headers
        Rectangle {
            width: parent.width
            height: 40
            color: "#2a2a2a"

            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    text: "Timestamp"
                    color: "#888888"
                    font.pixelSize: 12
                    font.bold: true
                    width: 140
                }

                Text {
                    text: "CAN ID"
                    color: "#888888"
                    font.pixelSize: 12
                    font.bold: true
                    width: 80
                }

                Text {
                    text: "DLC"
                    color: "#888888"
                    font.pixelSize: 12
                    font.bold: true
                    width: 50
                }

                Text {
                    text: "Data (Hex)"
                    color: "#888888"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }

        // Raw CAN data list - using snapshot, not live data
        Rectangle {
            width: parent.width
            height: parent.height - 180
            color: "#0a0a0a"

            ListView {
                id: rawDataListView
                anchors.fill: parent
                clip: true

                model: snapshotMessages

                delegate: Rectangle {
                    width: rawDataListView.width
                    height: 35
                    color: index % 2 === 0 ? "#1a1a1a" : "#151515"

                    Row {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        // Timestamp
                        Text {
                            text: modelData.timestamp || ""
                            color: "#666666"
                            font.pixelSize: 11
                            font.family: "Courier"
                            width: 140
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // CAN ID
                        Text {
                            text: modelData.canId || ""
                            color: "#4ecdc4"
                            font.pixelSize: 12
                            font.family: "Courier"
                            font.bold: true
                            width: 80
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // DLC (Data Length Code)
                        Text {
                            text: modelData.dlc !== undefined ? modelData.dlc.toString() : ""
                            color: "#ff9900"
                            font.pixelSize: 12
                            font.family: "Courier"
                            width: 50
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // Data bytes
                        Text {
                            text: modelData.data || ""
                            color: "#00ff00"
                            font.pixelSize: 12
                            font.family: "Courier"
                            elide: Text.ElideRight
                            width: parent.width - 280
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                }
            }

            // Info text when no messages
            Text {
                visible: snapshotMessages.length === 0
                text: "No CAN messages captured yet.\nAuto-refresh is " + (autoRefresh ? "ON" : "OFF")
                color: "#888888"
                font.pixelSize: 16
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
