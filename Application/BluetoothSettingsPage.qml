import QtQuick
import QtQuick.Controls

Rectangle {
    id: bluetoothSettingsPage
    color: "#1a1a1a"

    // ── Header ────────────────────────────────────────────────────────────────
    Rectangle {
        id: header
        width: parent.width
        height: 56
        color: "#222222"
        z: 10

        Text {
            text: "Bluetooth"
            color: "white"
            font.pixelSize: 17
            font.bold: true
            anchors.centerIn: parent
        }
    }

    // ── Content ───────────────────────────────────────────────────────────────
    Column {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        anchors.margins: 0
        spacing: 0

        // ── Power + Status row ────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 70
            color: "#222222"
            border.color: "#2a2a2a"
            border.width: 1

            // Text column — left side
            Column {
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Text {
                    text: "Bluetooth"
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }
                Text {
                    text: bluetoothManager.powered
                        ? (bluetoothManager.connected
                           ? "Connected to " + bluetoothManager.connectedName
                           : "On — not connected")
                        : "Off"
                    color: "#888888"
                    font.pixelSize: 12
                }
            }

            // Power toggle switch — right side
            Item {
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                width: 50
                height: 28

                Rectangle {
                    id: toggleTrack
                    anchors.fill: parent
                    radius: 14
                    color: bluetoothManager.powered ? "#4a6fc7" : "#444444"

                    Behavior on color { ColorAnimation { duration: 200 } }

                    Rectangle {
                        id: toggleKnob
                        width: 22
                        height: 22
                        radius: 11
                        color: "white"
                        anchors.verticalCenter: parent.verticalCenter
                        x: bluetoothManager.powered ? parent.width - width - 3 : 3

                        Behavior on x { NumberAnimation { duration: 200 } }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: bluetoothManager.setPower(!bluetoothManager.powered)
                }
            }
        }

        // ── Paired Devices ────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 40
            color: "#111111"

            Text {
                text: "MY DEVICES"
                color: "#555d72"
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 2
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: bluetoothManager.pairedDevices.length + " / 5"
                color: "#444444"
                font.pixelSize: 11
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Paired device list
        ListView {
            id: deviceList
            width: parent.width
            height: Math.min(contentHeight, 240)
            model: bluetoothManager.pairedDevices
            clip: true

            delegate: Rectangle {
                width: deviceList.width
                height: 68
                color: "#1a1a1a"

                // Bottom divider
                Rectangle {
                    width: parent.width - 40
                    height: 1
                    color: "#2a2a2a"
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                }

                // Avatar
                Rectangle {
                    width: 40
                    height: 40
                    radius: 20
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                    color: modelData.connected ? "#1a3a6a" : "#2a2a2a"
                    border.color: modelData.connected ? "#4a6fc7" : "#3a3a3a"
                    border.width: 1
                    Text { anchors.centerIn: parent; text: "📱"; font.pixelSize: 18 }
                }

                // Name + status
                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 70
                    anchors.right: parent.right
                    anchors.rightMargin: 130
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Text {
                        text: modelData.name
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        elide: Text.ElideRight
                        width: parent.width
                    }
                    Rectangle {
                        width: statusText.width + 12
                        height: 18
                        radius: 9
                        color: modelData.connected ? "#1a4a1a" : "transparent"
                        Text {
                            id: statusText
                            anchors.centerIn: parent
                            text: modelData.connected ? "Connected" : "Paired"
                            color: modelData.connected ? "#4aff4a" : "#666666"
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                }

                // Action buttons
                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    Rectangle {
                        width: 80
                        height: 30
                        radius: 8
                        color: modelData.connected ? "#3a1a1a" : "#1a3a1a"
                        border.color: modelData.connected ? "#7a2a2a" : "#2a7a2a"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: modelData.connected ? "Disconnect" : "Connect"
                            color: modelData.connected ? "#ff6b6b" : "#6bff8a"
                            font.pixelSize: 10
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (modelData.connected)
                                    bluetoothManager.disconnectDevice(modelData.address)
                                else
                                    bluetoothManager.connectDevice(modelData.address)
                            }
                        }
                    }

                    Rectangle {
                        width: 30
                        height: 30
                        radius: 8
                        color: "#2a2a2a"
                        border.color: "#3a3a3a"
                        border.width: 1
                        Text { anchors.centerIn: parent; text: "✕"; color: "#888888"; font.pixelSize: 13 }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: removeConfirm.show(modelData.address, modelData.name)
                        }
                    }
                }
            }
        }

        // ── Scan for new devices ──────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 40
            color: "#111111"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                text: "NEW DEVICES"
                color: "#555d72"
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 2
            }

            // Scan button
            Rectangle {
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                width: 80
                height: 26
                radius: 8
                color: bluetoothManager.discovering ? "#3a2a10" : "#1e2230"
                border.color: bluetoothManager.discovering ? "#c77a20" : "#4a6fc7"
                border.width: 1
                visible: bluetoothManager.powered
                         && bluetoothManager.pairedDevices.length < 5

                Text {
                    anchors.centerIn: parent
                    text: bluetoothManager.discovering ? "Stop" : "Scan"
                    color: bluetoothManager.discovering ? "#ffaa44" : "#7ab3ff"
                    font.pixelSize: 11
                    font.bold: true
                }

                // Pulsing animation while scanning
                SequentialAnimation on opacity {
                    running: bluetoothManager.discovering
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.5; duration: 800 }
                    NumberAnimation { to: 1.0; duration: 800 }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (bluetoothManager.discovering)
                            bluetoothManager.stopDiscovery()
                        else
                            bluetoothManager.startDiscovery()
                    }
                }
            }
        }

        // Discovered devices list
        ListView {
            id: discoveredList
            width: parent.width
            height: Math.min(contentHeight, 200)
            model: discoveredModel
            clip: true
            visible: bluetoothManager.discovering || count > 0

            delegate: Rectangle {
                width: discoveredList.width
                height: 60
                color: "#1a1a1a"

                Rectangle {
                    width: parent.width - 40
                    height: 1
                    color: "#2a2a2a"
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                }

                Row {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    Rectangle {
                        width: 36
                        height: 36
                        radius: 18
                        color: "#2a2a2a"
                        anchors.verticalCenter: parent.verticalCenter
                        Text { anchors.centerIn: parent; text: "📡"; font.pixelSize: 16 }
                    }

                    Text {
                        text: model.name || model.address
                        color: "white"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 120
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        width: 54
                        height: 28
                        radius: 8
                        color: "#1a3a1a"
                        border.color: "#2a6a2a"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "Pair"
                            color: "#6bff8a"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: bluetoothManager.connectDevice(model.address)
                        }
                    }
                }
            }
        }

        // Scanning spinner
        Item {
            width: parent.width
            height: 60
            visible: bluetoothManager.discovering && discoveredList.count === 0

            Row {
                anchors.centerIn: parent
                spacing: 10

                Rectangle {
                    width: 8; height: 8; radius: 4; color: "#7ab3ff"
                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.2; duration: 400 }
                        NumberAnimation { to: 1.0; duration: 400 }
                    }
                }
                Rectangle {
                    width: 8; height: 8; radius: 4; color: "#7ab3ff"
                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.2; duration: 400; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 1.0; duration: 400 }
                    }
                    Component.onCompleted: { }
                }
                Rectangle {
                    width: 8; height: 8; radius: 4; color: "#7ab3ff"
                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.2; duration: 400 }
                        NumberAnimation { to: 1.0; duration: 400 }
                    }
                }

                Text {
                    text: "Scanning for devices..."
                    color: "#7ab3ff"
                    font.pixelSize: 13
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // ── Discovered devices model ──────────────────────────────────────────────
    ListModel { id: discoveredModel }

    Connections {
        target: bluetoothManager
        function onDeviceFound(address, name) {
            // Don't add if already paired
            for (var d of bluetoothManager.pairedDevices)
                if (d.address === address) return
            // Don't add duplicates
            for (var i = 0; i < discoveredModel.count; i++)
                if (discoveredModel.get(i).address === address) return
            discoveredModel.append({ address: address, name: name })
        }
        function onDiscoveringChanged(discovering) {
            if (!discovering) discoveredModel.clear()
        }
    }

    // ── Remove confirmation dialog ────────────────────────────────────────────
    Rectangle {
        id: removeConfirm
        anchors.fill: parent
        color: "#aa000000"
        visible: false
        z: 100

        property string pendingAddress: ""
        property string pendingName: ""

        function show(address, name) {
            pendingAddress = address
            pendingName = name
            visible = true
        }

        Rectangle {
            anchors.centerIn: parent
            width: 300
            height: 140
            radius: 14
            color: "#222222"
            border.color: "#333333"
            border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 16
                width: parent.width - 40

                Text {
                    text: 'Remove "' + removeConfirm.pendingName + '" ?'
                    color: "white"
                    font.pixelSize: 14
                    font.bold: true
                    width: parent.width
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        width: 110
                        height: 36
                        radius: 8
                        color: "#2a2a2a"
                        Text {
                            anchors.centerIn: parent
                            text: "Cancel"
                            color: "#aaaaaa"
                            font.pixelSize: 13
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: removeConfirm.visible = false
                        }
                    }

                    Rectangle {
                        width: 110
                        height: 36
                        radius: 8
                        color: "#3a1a1a"
                        border.color: "#7a2a2a"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "Remove"
                            color: "#ff6b6b"
                            font.pixelSize: 13
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                bluetoothManager.removeDevice(removeConfirm.pendingAddress)
                                removeConfirm.visible = false
                            }
                        }
                    }
                }
            }
        }

        MouseArea { anchors.fill: parent; onClicked: removeConfirm.visible = false }
    }
}
