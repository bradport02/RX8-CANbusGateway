import QtQuick
import QtQuick.Controls

Rectangle {
    id: bluetoothSettingsPage
    color: "#1a1a1a"

    // Track which address is currently being paired — drives Pair button feedback
    property string pairingAddress: ""

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
        spacing: 0

        // ── Power + Status row ────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 70
            color: "#222222"
            border.color: "#2a2a2a"
            border.width: 1

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

            Item {
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                width: 50; height: 28

                Rectangle {
                    anchors.fill: parent
                    radius: 14
                    color: bluetoothManager.powered ? "#4a6fc7" : "#444444"
                    Behavior on color { ColorAnimation { duration: 200 } }

                    Rectangle {
                        width: 22; height: 22; radius: 11
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

        // ── MY DEVICES header ─────────────────────────────────────────────────
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

        // ── Paired device list ────────────────────────────────────────────────
        ListView {
            id: deviceList
            width: parent.width
            height: Math.min(contentHeight, 240)
            model: bluetoothManager.pairedDevices
            clip: true

            delegate: Rectangle {
                id: deviceRow
                width: deviceList.width
                height: 68
                color: "#1a1a1a"

                // Read directly from bluetoothManager so connected state stays live
                // after reconnect without needing modelData to deep-update
                readonly property string devAddress: modelData ? (modelData["address"] || "") : ""
                readonly property string devName:    modelData ? (modelData["name"]    || "") : ""

                // Re-check connected state from the live manager, not cached modelData
                readonly property bool devConnected: {
                    if (devAddress === "") return false
                    var devices = bluetoothManager.pairedDevices
                    for (var i = 0; i < devices.length; i++) {
                        if (devices[i]["address"] === devAddress)
                            return devices[i]["connected"] || false
                    }
                    return false
                }

                Rectangle {
                    width: parent.width - 40; height: 1
                    color: "#2a2a2a"
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                }

                // Avatar circle
                Rectangle {
                    width: 40; height: 40; radius: 20
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                    color: devConnected ? "#1a3a6a" : "#2a2a2a"
                    border.color: devConnected ? "#4a6fc7" : "#3a3a3a"
                    border.width: 1
                    Text { anchors.centerIn: parent; text: "📱"; font.pixelSize: 18 }
                }

                // Name + status badge
                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 70
                    anchors.right: parent.right
                    anchors.rightMargin: 190
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Text {
                        text: devName
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        elide: Text.ElideRight
                        width: parent.width
                    }
                    Rectangle {
                        width: statusText.width + 12
                        height: 18; radius: 9
                        color: devConnected ? "#1a4a1a" : "transparent"
                        Text {
                            id: statusText
                            anchors.centerIn: parent
                            text: devConnected ? "Connected" : "Paired"
                            color: devConnected ? "#4aff4a" : "#666666"
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
                        width: 90; height: 30; radius: 8
                        color: devConnected ? "#3a1a1a" : "#1a3a1a"
                        border.color: devConnected ? "#7a2a2a" : "#2a7a2a"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: devConnected ? "Disconnect" : "Connect"
                            color: devConnected ? "#ff6b6b" : "#6bff8a"
                            font.pixelSize: 10
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (devConnected)
                                    bluetoothManager.disconnectDevice(devAddress)
                                else
                                    bluetoothManager.connectDevice(devAddress)
                            }
                        }
                    }

                    Rectangle {
                        width: 70; height: 30; radius: 8
                        color: "#3a1a1a"
                        border.color: "#7a2a2a"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "Remove"
                            color: "#ff6b6b"
                            font.pixelSize: 10
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: removeConfirm.show(devAddress, devName)
                        }
                    }
                }
            }
        }

        // ── Scan + Discoverable buttons ───────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 58
            color: "#111111"
            visible: bluetoothManager.powered

            Row {
                anchors.centerIn: parent
                spacing: 12

                // Scan button
                Rectangle {
                    width: 180; height: 36; radius: 8
                    color: bluetoothManager.discovering ? "#1a2a4a" : "#1e2035"
                    border.color: bluetoothManager.discovering ? "#4a6fc7" : "#333355"
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 8

                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: "#4a6fc7"
                            visible: bluetoothManager.discovering
                            anchors.verticalCenter: parent.verticalCenter
                            SequentialAnimation on opacity {
                                running: bluetoothManager.discovering
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.2; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }

                        Text {
                            text: bluetoothManager.discovering ? "Stop Scanning" : "Scan for Devices"
                            color: bluetoothManager.discovering ? "#7ab3ff" : "#aaaacc"
                            font.pixelSize: 12
                            font.bold: bluetoothManager.discovering
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: bluetoothManager.discovering
                                   ? bluetoothManager.stopDiscovery()
                                   : bluetoothManager.startDiscovery()
                    }
                }

                // Discoverable button — 30s countdown driven by backend
                Rectangle {
                    id: discoverableBtn
                    width: 180; height: 36; radius: 8
                    color: bluetoothManager.discoverable ? "#1a3a2a" : "#1e2035"
                    border.color: bluetoothManager.discoverable ? "#2a7a4a" : "#333355"
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 8

                        // Pulsing dot when discoverable
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: "#2aff7a"
                            visible: bluetoothManager.discoverable
                            anchors.verticalCenter: parent.verticalCenter
                            SequentialAnimation on opacity {
                                running: bluetoothManager.discoverable
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.2; duration: 600 }
                                NumberAnimation { to: 1.0; duration: 600 }
                            }
                        }

                        Text {
                            text: bluetoothManager.discoverable
                                  ? "Visible  " + bluetoothManager.discoverableSeconds + "s"
                                  : "Make Discoverable"
                            color: bluetoothManager.discoverable ? "#2aff7a" : "#aaaacc"
                            font.pixelSize: 12
                            font.bold: bluetoothManager.discoverable
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: bluetoothManager.makeDiscoverable(30)
                    }
                }
            }
        }

        // ── NEW DEVICES header ────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 40
            color: "#111111"
            visible: bluetoothManager.discovering || discoveredList.count > 0

            Text {
                text: "NEW DEVICES"
                color: "#555d72"
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 2
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // ── Discovered devices list ───────────────────────────────────────────
        ListView {
            id: discoveredList
            width: parent.width
            height: Math.min(contentHeight, 200)
            model: discoveredModel
            clip: true
            visible: discoveredList.count > 0

            delegate: Rectangle {
                width: discoveredList.width
                height: 60
                color: "#1a1a1a"

                readonly property string discAddress: model.address || ""
                readonly property string discName:    model.name    || ""
                readonly property bool   isPairing:   bluetoothSettingsPage.pairingAddress === discAddress

                Rectangle {
                    width: parent.width - 40; height: 1
                    color: "#2a2a2a"
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                }

                // Icon
                Rectangle {
                    width: 36; height: 36; radius: 18
                    color: "#2a2a2a"
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    Text { anchors.centerIn: parent; text: "📡"; font.pixelSize: 16 }
                }

                // Device name
                Text {
                    text: discName || discAddress
                    color: "white"
                    font.pixelSize: 14
                    anchors.left: parent.left
                    anchors.leftMargin: 62
                    anchors.right: parent.right
                    anchors.rightMargin: 80
                    anchors.verticalCenter: parent.verticalCenter
                    elide: Text.ElideRight
                }

                // Pair button — shows spinner while pairing
                Rectangle {
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    width: 70; height: 30; radius: 8
                    color: isPairing ? "#1a2a4a" : "#1a3a1a"
                    border.color: isPairing ? "#4a6fc7" : "#2a6a2a"
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 6

                        // Animated dot when pairing
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: "#7ab3ff"
                            visible: isPairing
                            anchors.verticalCenter: parent.verticalCenter
                            SequentialAnimation on opacity {
                                running: isPairing
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.2; duration: 400 }
                                NumberAnimation { to: 1.0; duration: 400 }
                            }
                        }

                        Text {
                            text: isPairing ? "Pairing..." : "Pair"
                            color: isPairing ? "#7ab3ff" : "#6bff8a"
                            font.pixelSize: 11
                            font.bold: isPairing
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: !isPairing
                        onClicked: {
                            bluetoothSettingsPage.pairingAddress = discAddress
                            bluetoothManager.connectDevice(discAddress)
                        }
                    }
                }
            }
        }

        // Scanning spinner (when discovering but no devices found yet)
        Item {
            width: parent.width
            height: 50
            visible: bluetoothManager.discovering && discoveredList.count === 0

            Row {
                anchors.centerIn: parent
                spacing: 8

                Repeater {
                    model: 3
                    Rectangle {
                        width: 7; height: 7; radius: 3.5; color: "#7ab3ff"
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.2; duration: 400; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 400; easing.type: Easing.InOutSine }
                            PauseAnimation { duration: index * 133 }
                        }
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
            for (var d of bluetoothManager.pairedDevices)
                if (d["address"] === address) return
            for (var i = 0; i < discoveredModel.count; i++)
                if (discoveredModel.get(i).address === address) return
            discoveredModel.append({ address: address, name: name })
        }

        function onDiscoveringChanged(discovering) {
            if (!discovering) discoveredModel.clear()
        }

        // When a device connects, clear pairingAddress so button resets
        function onConnectedChanged() {
            bluetoothSettingsPage.pairingAddress = ""
        }

        // Also clear pairing state on error
        function onErrorOccurred(message) {
            bluetoothSettingsPage.pairingAddress = ""
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

        // Background dismiss — FIRST child so dialog sits on top
        MouseArea {
            anchors.fill: parent
            onClicked: removeConfirm.visible = false
        }

        Rectangle {
            anchors.centerIn: parent
            width: 300; height: 140; radius: 14
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
                        width: 110; height: 36; radius: 8
                        color: "#2a2a2a"
                        Text { anchors.centerIn: parent; text: "Cancel"; color: "#aaaaaa"; font.pixelSize: 13 }
                        MouseArea { anchors.fill: parent; onClicked: removeConfirm.visible = false }
                    }

                    Rectangle {
                        width: 110; height: 36; radius: 8
                        color: "#3a1a1a"
                        border.color: "#7a2a2a"; border.width: 1
                        Text { anchors.centerIn: parent; text: "Remove"; color: "#ff6b6b"; font.pixelSize: 13; font.bold: true }
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
    }
}
