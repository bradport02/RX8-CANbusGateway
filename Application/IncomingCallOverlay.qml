import QtQuick
import QtQuick.Controls

// ── IncomingCallOverlay.qml ───────────────────────────────────────────────
// Place this at the TOP LEVEL of your main window, above all other content.
// It will appear as a slide-in card whenever an incoming call is detected.
//
// Usage in your main QML:
//   IncomingCallOverlay { id: callOverlay; anchors.fill: parent; z: 1000 }

Item {
    id: callOverlay
    anchors.fill: parent
    visible: callManager.callState === 1 ||  // Incoming
             callManager.callState === 2 ||  // Active
             callManager.callState === 3     // Dialling
    z: 1000

    // ── Background dim ─────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: "#cc000000"
        visible: callManager.callState === 1  // Only dim on incoming
        opacity: visible ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 300 } }
    }

    // ── Active / Dialling call bar (compact, top of screen) ────────────────
    Rectangle {
        id: activeCallBar
        width: parent.width
        height: 52
        color: "#1a3a1a"
        border.color: "#2a6a2a"
        border.width: 1
        visible: callManager.callState === 2 || callManager.callState === 3
        y: visible ? 0 : -height

        Behavior on y { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }

        Row {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            // Animated ring
            Rectangle {
                width: 28
                height: 28
                radius: 14
                color: "#2a6a2a"
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    anchors.centerIn: parent
                    text: callManager.callState === 3 ? "📱" : "📞"
                    font.pixelSize: 14
                }

                SequentialAnimation on scale {
                    running: callManager.callState === 2
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.1; duration: 800 }
                    NumberAnimation { to: 1.0; duration: 800 }
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2

                Text {
                    text: callManager.callerName || callManager.callerNumber || "Unknown"
                    color: "white"
                    font.pixelSize: 13
                    font.bold: true
                }
                Text {
                    text: callManager.callState === 3
                        ? "Calling..."
                        : formatDuration(callManager.callDuration)
                    color: "#88ff88"
                    font.pixelSize: 11
                    font.family: "Courier"
                }
            }

            // Hang up button (compact)
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                width: 60
                height: 32
                radius: 16
                color: "#3a1a1a"
                border.color: "#ff4444"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "End"
                    color: "#ff6b6b"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: callManager.hangup()
                }
            }
        }
    }

    // ── Incoming call card ──────────────────────────────────────────────────
    Rectangle {
        id: incomingCard
        width: Math.min(parent.width - 40, 380)
        height: 260
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -20
        radius: 20
        color: "#1e1e2e"
        border.color: "#3a3a5a"
        border.width: 1
        visible: callManager.callState === 1

        scale: visible ? 1.0 : 0.85
        Behavior on scale { NumberAnimation { duration: 300; easing.type: Easing.OutBack } }

        // Glow ring around card
        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 16
            height: parent.height + 16
            radius: parent.radius + 8
            color: "transparent"
            border.color: "#4a4a8a"
            border.width: 2
            opacity: 0.5

            SequentialAnimation on opacity {
                running: callManager.callState === 1
                loops: Animation.Infinite
                NumberAnimation { to: 0.1; duration: 1000 }
                NumberAnimation { to: 0.5; duration: 1000 }
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 0

            // Incoming label
            Text {
                text: "INCOMING CALL"
                color: "#7ab3ff"
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 3
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item { height: 20; width: 1 }

            // Avatar circle
            Rectangle {
                width: 80
                height: 80
                radius: 40
                anchors.horizontalCenter: parent.horizontalCenter
                color: Qt.hsla(
                    (callManager.callerName.charCodeAt(0) * 137) % 360 / 360,
                    0.4, 0.25, 1)
                border.color: Qt.hsla(
                    (callManager.callerName.charCodeAt(0) * 137) % 360 / 360,
                    0.6, 0.5, 1)
                border.width: 2

                // Ripple ring
                Rectangle {
                    anchors.centerIn: parent
                    width: 80
                    height: 80
                    radius: 40
                    color: "transparent"
                    border.color: Qt.hsla(
                        (callManager.callerName.charCodeAt(0) * 137) % 360 / 360,
                        0.5, 0.5, 0.4)
                    border.width: 3

                    SequentialAnimation on scale {
                        running: callManager.callState === 1
                        loops: Animation.Infinite
                        NumberAnimation { to: 1.6; duration: 1200; easing.type: Easing.OutCubic }
                        NumberAnimation { to: 1.0; duration: 0 }
                    }
                    SequentialAnimation on opacity {
                        running: callManager.callState === 1
                        loops: Animation.Infinite
                        NumberAnimation { to: 0; duration: 1200 }
                        NumberAnimation { to: 1; duration: 0 }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: {
                        var name = callManager.callerName || callManager.callerNumber
                        if (!name) return "?"
                        var parts = name.split(" ")
                        return parts.length > 1
                            ? parts[0][0].toUpperCase() + parts[1][0].toUpperCase()
                            : name[0].toUpperCase()
                    }
                    color: Qt.hsla(
                        (callManager.callerName.charCodeAt(0) * 137) % 360 / 360,
                        0.9, 0.8, 1)
                    font.pixelSize: 28
                    font.bold: true
                }
            }

            Item { height: 16; width: 1 }

            // Caller name
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: callManager.callerName || "Unknown Caller"
                color: "white"
                font.pixelSize: 20
                font.bold: true
                elide: Text.ElideRight
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
            }

            Item { height: 4; width: 1 }

            // Caller number
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: callManager.callerNumber
                color: "#888888"
                font.pixelSize: 14
                font.family: "Courier"
            }

            Item { height: 20; width: 1 }

            // Accept / Decline buttons
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 32

                // Decline
                Column {
                    spacing: 8
                    Rectangle {
                        width: 64
                        height: 64
                        radius: 32
                        color: "#3a1a1a"
                        border.color: "#ff4444"
                        border.width: 2
                        anchors.horizontalCenter: parent.horizontalCenter

                        Text {
                            anchors.centerIn: parent
                            text: "📵"
                            font.pixelSize: 28
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: callManager.declineCall()
                        }
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Decline"
                        color: "#ff6b6b"
                        font.pixelSize: 11
                    }
                }

                // Accept
                Column {
                    spacing: 8
                    Rectangle {
                        width: 64
                        height: 64
                        radius: 32
                        color: "#1a3a1a"
                        border.color: "#44ff44"
                        border.width: 2
                        anchors.horizontalCenter: parent.horizontalCenter

                        Text {
                            anchors.centerIn: parent
                            text: "📞"
                            font.pixelSize: 28
                        }

                        // Pulse animation
                        SequentialAnimation on scale {
                            running: callManager.callState === 1
                            loops: Animation.Infinite
                            NumberAnimation { to: 1.08; duration: 600 }
                            NumberAnimation { to: 1.0;  duration: 600 }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: callManager.acceptCall()
                        }
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Accept"
                        color: "#6bff8a"
                        font.pixelSize: 11
                    }
                }
            }
        }
    }

    // ── Helpers ────────────────────────────────────────────────────────────────
    function formatDuration(secs) {
        var m = Math.floor(secs / 60)
        var s = secs % 60
        return (m < 10 ? "0" + m : m) + ":" + (s < 10 ? "0" + s : s)
    }
}
