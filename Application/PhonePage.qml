import QtQuick
import QtQuick.Controls

Rectangle {
    id: phonePage
    color: "#1a1a1a"

    // ── Tab state ────────────────────────────────────────────────────────────
    property int activeTab: 0   // 0 = contacts, 1 = dialpad

    // ── Header ───────────────────────────────────────────────────────────────
    Rectangle {
        id: header
        width: parent.width
        height: 56
        color: "#222222"
        z: 10

        Text {
            text: "Phone"
            color: "white"
            font.pixelSize: 17
            font.bold: true
            anchors.centerIn: parent
        }

        // Sync button
        Rectangle {
            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            width: 60
            height: 28
            radius: 8
            color: contactsManager.syncing ? "#2a2a00" : "#1e2230"
            border.color: contactsManager.syncing ? "#aaaa00" : "#4a6fc7"
            border.width: 1
            visible: bluetoothManager.connected

            Text {
                anchors.centerIn: parent
                text: contactsManager.syncing ? "Syncing" : "Sync"
                color: contactsManager.syncing ? "#ffff44" : "#7ab3ff"
                font.pixelSize: 10
                font.bold: true
            }

            SequentialAnimation on opacity {
                running: contactsManager.syncing
                loops: Animation.Infinite
                NumberAnimation { to: 0.4; duration: 600 }
                NumberAnimation { to: 1.0; duration: 600 }
            }

            MouseArea {
                anchors.fill: parent
                enabled: !contactsManager.syncing && bluetoothManager.connected
                onClicked: contactsManager.syncContacts(bluetoothManager.connectedName)
            }
        }
    }

    // ── Tab bar ───────────────────────────────────────────────────────────────
    Rectangle {
        id: tabBar
        anchors.top: header.bottom
        width: parent.width
        height: 44
        color: "#1a1a1a"
        z: 5

        Row {
            anchors.fill: parent

            Repeater {
                model: ["Contacts", "Dial Pad"]
                delegate: Rectangle {
                    width: tabBar.width / 2
                    height: tabBar.height
                    color: activeTab === index ? "#1e2230" : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: activeTab === index ? "#7ab3ff" : "#666666"
                        font.pixelSize: 13
                        font.bold: activeTab === index
                    }

                    // Active indicator bar
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 40
                        height: 2
                        radius: 1
                        color: "#4a6fc7"
                        visible: activeTab === index
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: activeTab = index
                    }
                }
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "#2a2a2a"
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // CONTACTS TAB
    // ══════════════════════════════════════════════════════════════════════════
    Item {
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        visible: activeTab === 0

        // Search bar
        Rectangle {
            id: searchBar
            width: parent.width - 32
            height: 36
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 10
            radius: 10
            color: "#222222"
            border.color: searchInput.activeFocus ? "#4a6fc7" : "#333333"
            border.width: 1

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: "🔍"
                font.pixelSize: 14
                color: "#666666"
            }

            TextInput {
                id: searchInput
                anchors.left: parent.left
                anchors.leftMargin: 36
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                font.pixelSize: 13
                onTextChanged: {
                    var results = contactsManager.search(text)
                    contactsModel.clear()
                    for (var c of results)
                        contactsModel.append(c)
                }

                Text {
                    anchors.fill: parent
                    text: "Search contacts..."
                    color: "#555555"
                    font: parent.font
                    visible: !parent.text && !parent.activeFocus
                }
            }
        }

        // Contact count
        Text {
            id: countLabel
            anchors.top: searchBar.bottom
            anchors.topMargin: 6
            anchors.left: parent.left
            anchors.leftMargin: 20
            text: contactsModel.count + " contacts"
            color: "#555d72"
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 1.5
        }

        // Contact list
        ListView {
            id: contactList
            anchors.top: countLabel.bottom
            anchors.topMargin: 6
            anchors.bottom: parent.bottom
            width: parent.width
            clip: true
            model: contactsModel

            // Group header component
            section.property: "name"
            section.criteria: ViewSection.FirstCharacter

            delegate: Rectangle {
                width: contactList.width
                height: 64
                color: contactMouse.pressed ? "#252525" : "#1a1a1a"

                Rectangle {
                    width: parent.width - 36
                    height: 1
                    color: "#222222"
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                }

                // Avatar
                Rectangle {
                    width: 42
                    height: 42
                    radius: 21
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    color: Qt.hsla((model.name.charCodeAt(0) * 137) % 360 / 360, 0.45, 0.28, 1)

                    Text {
                        anchors.centerIn: parent
                        text: model.initials || "?"
                        color: Qt.hsla((model.name.charCodeAt(0) * 137) % 360 / 360, 0.8, 0.75, 1)
                        font.pixelSize: 15
                        font.bold: true
                    }
                }

                // Name + number
                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 74
                    anchors.right: parent.right
                    anchors.rightMargin: 62
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Text {
                        text: model.name
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        elide: Text.ElideRight
                        width: parent.width
                    }
                    Text {
                        text: model.number
                        color: "#888888"
                        font.pixelSize: 12
                        font.family: "Courier"
                    }
                }

                // Call button
                Rectangle {
                    width: 40
                    height: 40
                    radius: 20
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#1a3a1a"
                    border.color: "#2a6a2a"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "📞"
                        font.pixelSize: 18
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: callManager.dial(model.number.replace(/[^\d+]/g, ""))
                    }
                }

                MouseArea {
                    id: contactMouse
                    anchors.fill: parent
                    onClicked: {
                        // Pre-fill dialpad
                        dialInput.text = model.number.replace(/[^\d+]/g, "")
                        activeTab = 1
                    }
                }
            }

            // Empty state
            Item {
                anchors.centerIn: parent
                visible: contactList.count === 0 && !contactsManager.syncing
                width: parent.width
                height: 120

                Column {
                    anchors.centerIn: parent
                    spacing: 12

                    Text {
                        text: bluetoothManager.connected ? "📋" : "📵"
                        font.pixelSize: 40
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: bluetoothManager.connected
                            ? "Tap Sync to load contacts"
                            : "Connect a phone first"
                        color: "#555555"
                        font.pixelSize: 14
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // DIAL PAD TAB
    // ══════════════════════════════════════════════════════════════════════════
    Item {
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        visible: activeTab === 1

        // Number display
        Rectangle {
            id: dialDisplay
            width: parent.width - 32
            height: 56
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 14
            radius: 12
            color: "#222222"
            border.color: "#333333"
            border.width: 1

            TextInput {
                id: dialInput
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                color: "white"
                font.pixelSize: 22
                font.family: "Courier"
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                readOnly: true

                Text {
                    anchors.centerIn: parent
                    text: "Enter number..."
                    color: "#444444"
                    font: parent.font
                    visible: !parent.text
                }
            }

            // Backspace
            Rectangle {
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                width: 36
                height: 36
                radius: 8
                color: "transparent"
                visible: dialInput.text.length > 0

                Text {
                    anchors.centerIn: parent
                    text: "⌫"
                    color: "#888888"
                    font.pixelSize: 20
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: dialInput.text = dialInput.text.slice(0, -1)
                    onPressAndHold: dialInput.text = ""
                }
            }
        }

        // Dial pad grid
        Grid {
            id: dialGrid
            anchors.top: dialDisplay.bottom
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            columns: 3
            spacing: 12

            Repeater {
                model: [
                    { digit: "1", sub: "" },    { digit: "2", sub: "ABC" }, { digit: "3", sub: "DEF" },
                    { digit: "4", sub: "GHI" }, { digit: "5", sub: "JKL" }, { digit: "6", sub: "MNO" },
                    { digit: "7", sub: "PQRS" },{ digit: "8", sub: "TUV" }, { digit: "9", sub: "WXYZ" },
                    { digit: "*", sub: "" },    { digit: "0", sub: "+" },   { digit: "#", sub: "" }
                ]

                delegate: Rectangle {
                    width: 82
                    height: 60
                    radius: 12
                    color: dialKeyMouse.pressed ? "#2a2a2a" : "#222222"
                    border.color: "#333333"
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 80 } }

                    Column {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.digit
                            color: "white"
                            font.pixelSize: 22
                            font.bold: true
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.sub
                            color: "#555555"
                            font.pixelSize: 9
                            font.bold: true
                            font.letterSpacing: 1.5
                            visible: modelData.sub.length > 0
                        }
                    }

                    MouseArea {
                        id: dialKeyMouse
                        anchors.fill: parent
                        onClicked: dialInput.text += modelData.digit
                        onPressAndHold: {
                            if (modelData.digit === "0") dialInput.text += "+"
                        }
                    }
                }
            }
        }

        // Call / Hang up button
        Rectangle {
            anchors.top: dialGrid.bottom
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            width: 82
            height: 60
            radius: 30
            color: {
                if (callManager.callState === 1 ||  // Incoming
                    callManager.callState === 2)     // Active
                    return "#3a1a1a"
                return "#1a3a1a"
            }
            border.color: {
                if (callManager.callState === 1 || callManager.callState === 2)
                    return "#ff4444"
                return "#44ff44"
            }
            border.width: 2

            Text {
                anchors.centerIn: parent
                text: (callManager.callState === 1 || callManager.callState === 2)
                    ? "📵" : "📞"
                font.pixelSize: 26
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (callManager.callState === 2)       // Active
                        callManager.hangup()
                    else if (dialInput.text.length > 0)
                        callManager.dial(dialInput.text.replace(/[^\d+]/g, ""))
                }
            }
        }
    }

    // ── Contacts model — populated from contactsManager ──────────────────────
    ListModel { id: contactsModel }

    Connections {
        target: contactsManager
        function onContactsLoaded() {
            contactsModel.clear()
            for (var c of contactsManager.contacts)
                contactsModel.append(c)
        }
    }

    Component.onCompleted: {
        uartController.sendLCDText("Telephone")
        // Load contacts on page open
        if (contactsManager.contactCount > 0) {
            contactsModel.clear()
            for (var c of contactsManager.contacts)
                contactsModel.append(c)
        }
    }
}
