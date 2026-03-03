import QtQuick
import QtQuick.Controls

Rectangle {
    id: activeCallPage
    color: "#111118"

    // ── Background gradient ───────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1a1a2e" }
            GradientStop { position: 1.0; color: "#111118" }
        }
    }

    // ── State: show dialpad overlay during active call ────────────────────────
    property bool showDtmf: false

    // ── Caller info ───────────────────────────────────────────────────────────
    Column {
        id: callerInfo
        anchors.top: parent.top
        anchors.topMargin: 48
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 12

        // Call state label
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: {
                switch (callManager.callState) {
                    case 1: return "INCOMING CALL"
                    case 2: return "ON CALL"
                    case 3: return "CALLING..."
                    default: return ""
                }
            }
            color: "#7ab3ff"
            font.pixelSize: 11
            font.bold: true
            font.letterSpacing: 3
        }

        // Avatar
        Rectangle {
            width: 100
            height: 100
            radius: 50
            anchors.horizontalCenter: parent.horizontalCenter
            color: Qt.hsla(
                (callManager.callerName.length > 0
                    ? callManager.callerName.charCodeAt(0) * 137 : 180) % 360 / 360,
                0.4, 0.25, 1)
            border.color: Qt.hsla(
                (callManager.callerName.length > 0
                    ? callManager.callerName.charCodeAt(0) * 137 : 180) % 360 / 360,
                0.6, 0.5, 1)
            border.width: 2

            // Pulse ring — only when incoming
            Rectangle {
                anchors.centerIn: parent
                width: 100; height: 100; radius: 50
                color: "transparent"
                border.color: Qt.hsla(
                    (callManager.callerName.length > 0
                        ? callManager.callerName.charCodeAt(0) * 137 : 180) % 360 / 360,
                    0.5, 0.5, 0.3)
                border.width: 3
                visible: callManager.callState === 1

                SequentialAnimation on scale {
                    running: callManager.callState === 1
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.7; duration: 1200; easing.type: Easing.OutCubic }
                    NumberAnimation { to: 1.0; duration: 0 }
                }
                SequentialAnimation on opacity {
                    running: callManager.callState === 1
                    loops: Animation.Infinite
                    NumberAnimation { to: 0; duration: 1200 }
                    NumberAnimation { to: 1; duration: 0 }
                }
            }

            // Initials
            Text {
                anchors.centerIn: parent
                text: {
                    var name = callManager.callerName || callManager.callerNumber
                    if (!name || name.length === 0) return "?"
                    var parts = name.split(" ")
                    return parts.length > 1
                        ? parts[0][0].toUpperCase() + parts[1][0].toUpperCase()
                        : name[0].toUpperCase()
                }
                color: "white"
                font.pixelSize: 36
                font.bold: true
            }
        }

        // Caller name
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: callManager.callerName || "Unknown Caller"
            color: "white"
            font.pixelSize: 24
            font.bold: true
        }

        // Caller number
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: callManager.callerNumber
            color: "#888888"
            font.pixelSize: 14
            font.family: "Courier"
            visible: callManager.callerName.length > 0
        }

        // Duration — only during active call
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: {
                var s = callManager.callDuration
                var m = Math.floor(s / 60)
                s = s % 60
                return (m < 10 ? "0" + m : m) + ":" + (s < 10 ? "0" + s : s)
            }
            color: "#44ff88"
            font.pixelSize: 18
            font.family: "Courier"
            font.bold: true
            visible: callManager.callState === 2  // Active only
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // INCOMING CALL BUTTONS
    // ══════════════════════════════════════════════════════════════════════════
    Row {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 48
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 60
        visible: callManager.callState === 1  // Incoming

        // Decline
        Column {
            spacing: 10
            Rectangle {
                width: 72; height: 72; radius: 36
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#3a1a1a"
                border.color: "#cc3333"; border.width: 2
                Text { anchors.centerIn: parent; text: "📵"; font.pixelSize: 32 }
                MouseArea {
                    anchors.fill: parent
                    onClicked: callManager.declineCall()
                }
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Decline"; color: "#ff6b6b"; font.pixelSize: 13
            }
        }

        // Accept
        Column {
            spacing: 10
            Rectangle {
                width: 72; height: 72; radius: 36
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#1a3a1a"
                border.color: "#33cc55"; border.width: 2
                Text { anchors.centerIn: parent; text: "📞"; font.pixelSize: 32 }
                SequentialAnimation on scale {
                    running: callManager.callState === 1
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.1; duration: 600 }
                    NumberAnimation { to: 1.0; duration: 600 }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: callManager.acceptCall()
                }
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Accept"; color: "#6bff8a"; font.pixelSize: 13
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // ACTIVE / DIALLING CALL CONTROLS
    // ══════════════════════════════════════════════════════════════════════════
    Column {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20
        visible: callManager.callState === 2 || callManager.callState === 3

        // ── Control buttons row ───────────────────────────────────────────────
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24

            // Mute
            Column {
                spacing: 8
                Rectangle {
                    width: 60; height: 60; radius: 30
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: callManager.muted ? "#3a2a00" : "#2a2a3a"
                    border.color: callManager.muted ? "#ffaa00" : "#4a4a6a"
                    border.width: 2
                    Text {
                        anchors.centerIn: parent
                        text: callManager.muted ? "🔇" : "🎤"
                        font.pixelSize: 26
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: callManager.setMute(!callManager.muted)
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: callManager.muted ? "Unmute" : "Mute"
                    color: callManager.muted ? "#ffaa44" : "#888888"
                    font.pixelSize: 11
                }
            }

            // Keypad / DTMF
            Column {
                spacing: 8
                Rectangle {
                    width: 60; height: 60; radius: 30
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: activeCallPage.showDtmf ? "#1a2a3a" : "#2a2a3a"
                    border.color: activeCallPage.showDtmf ? "#4a8aff" : "#4a4a6a"
                    border.width: 2
                    Text { anchors.centerIn: parent; text: "⌨️"; font.pixelSize: 26 }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: activeCallPage.showDtmf = !activeCallPage.showDtmf
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Keypad"; color: "#888888"; font.pixelSize: 11
                }
            }

            // Volume down
            Column {
                spacing: 8
                Rectangle {
                    width: 60; height: 60; radius: 30
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#2a2a3a"
                    border.color: "#4a4a6a"; border.width: 2
                    Text { anchors.centerIn: parent; text: "🔉"; font.pixelSize: 26 }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: callManager.setVolume(callManager.volume - 10)
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Vol -"; color: "#888888"; font.pixelSize: 11
                }
            }

            // Volume up
            Column {
                spacing: 8
                Rectangle {
                    width: 60; height: 60; radius: 30
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#2a2a3a"
                    border.color: "#4a4a6a"; border.width: 2
                    Text { anchors.centerIn: parent; text: "🔊"; font.pixelSize: 26 }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: callManager.setVolume(callManager.volume + 10)
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Vol +"; color: "#888888"; font.pixelSize: 11
                }
            }
        }

        // Volume bar
        Rectangle {
            width: 280
            height: 4
            radius: 2
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#2a2a2a"

            Rectangle {
                width: parent.width * (callManager.volume / 100)
                height: parent.height
                radius: parent.radius
                color: "#4a8aff"
                Behavior on width { NumberAnimation { duration: 150 } }
            }

            Text {
                anchors.right: parent.right
                anchors.bottom: parent.top
                anchors.bottomMargin: 4
                text: callManager.volume + "%"
                color: "#555555"
                font.pixelSize: 10
            }
        }

        // ── End call button ───────────────────────────────────────────────────
        Rectangle {
            width: 180; height: 56; radius: 28
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#3a1010"
            border.color: "#cc2222"; border.width: 2

            Row {
                anchors.centerIn: parent
                spacing: 10
                Text { text: "📵"; font.pixelSize: 22; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: "End Call"
                    color: "#ff6b6b"
                    font.pixelSize: 16
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: callManager.hangup()
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    // DTMF KEYPAD OVERLAY
    // ══════════════════════════════════════════════════════════════════════════
    Rectangle {
        anchors.fill: parent
        color: "#ee111118"
        visible: activeCallPage.showDtmf
        z: 10

        Column {
            anchors.centerIn: parent
            spacing: 16

            // DTMF input display
            Rectangle {
                width: 280; height: 48; radius: 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#1e1e2e"
                border.color: "#333355"; border.width: 1

                Text {
                    id: dtmfDisplay
                    anchors.centerIn: parent
                    text: ""
                    color: "white"
                    font.pixelSize: 20
                    font.family: "Courier"
                    font.bold: true
                }
            }

            // Keypad grid
            Grid {
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 3
                spacing: 12

                Repeater {
                    model: [
                        {d:"1",s:""},{d:"2",s:"ABC"},{d:"3",s:"DEF"},
                        {d:"4",s:"GHI"},{d:"5",s:"JKL"},{d:"6",s:"MNO"},
                        {d:"7",s:"PQRS"},{d:"8",s:"TUV"},{d:"9",s:"WXYZ"},
                        {d:"*",s:""},{d:"0",s:"+"},{d:"#",s:""}
                    ]

                    delegate: Rectangle {
                        width: 82; height: 56; radius: 10
                        color: dtmfKey.pressed ? "#2a2a3a" : "#1e1e2e"
                        border.color: "#333355"; border.width: 1

                        Column {
                            anchors.centerIn: parent
                            spacing: 2
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.d; color: "white"
                                font.pixelSize: 20; font.bold: true
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.s; color: "#555555"
                                font.pixelSize: 8; font.bold: true
                                font.letterSpacing: 1.5
                                visible: modelData.s.length > 0
                            }
                        }

                        MouseArea {
                            id: dtmfKey
                            anchors.fill: parent
                            onClicked: {
                                dtmfDisplay.text += modelData.d
                                callManager.sendDtmf(modelData.d)
                            }
                        }
                    }
                }
            }

            // Close keypad
            Rectangle {
                width: 120; height: 40; radius: 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#2a2a3a"
                border.color: "#4a4a6a"; border.width: 1
                Text { anchors.centerIn: parent; text: "Close"; color: "#aaaaaa"; font.pixelSize: 13 }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        activeCallPage.showDtmf = false
                        dtmfDisplay.text = ""
                    }
                }
            }
        }
    }

    // Auto-navigate back when call ends
    Connections {
        target: callManager
        function onCallEnded() {
            activeCallPage.showDtmf = false
        }
    }
}
