import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "transparent"

    // ── State — updated from UART incoming data (CMD 0x11) ────────────────────
    property int    activeCd:       1       // currently loaded CD (1-6)
    property int    currentTrack:   1
    property int    totalTracks:    0
    property int    trackLengthSec: 0       // total track length in seconds
    property int    currentTimeSec: 0       // playback position in seconds
    property bool   isPlaying:      false
    property bool   cdReady:        false

    property real progress: trackLengthSec > 0
                            ? Math.min(currentTimeSec / trackLengthSec, 1.0) : 0.0

    function formatTime(secs) {
        var m = Math.floor(secs / 60)
        var s = secs % 60
        return m + ":" + (s < 10 ? "0" : "") + s
    }

    // ── Upper half: disc info + vinyl ─────────────────────────────────────────
    Item {
        id: upperArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: lowerArea.top

        // Vinyl disc — right side
        Rectangle {
            id: discBox
            width: 180; height: 180
            anchors.right: parent.right
            anchors.rightMargin: 48
            anchors.verticalCenter: parent.verticalCenter
            radius: 90
            color: "#0d0d1a"
            border.color: cdReady ? "#4a6fc7" : "#2a2a4a"
            border.width: 2

            RotationAnimation on rotation {
                running: isPlaying && cdReady
                loops: Animation.Infinite
                duration: 3500
                from: 0; to: 360
            }

            // Groove rings
            Repeater {
                model: 5
                Rectangle {
                    property int r: 82 - index * 14
                    width: r*2; height: r*2; radius: r
                    anchors.centerIn: parent
                    color: "transparent"
                    border.color: "#1a1a30"
                    border.width: 1
                }
            }

            // Centre hub
            Rectangle {
                width: 50; height: 50; radius: 25
                anchors.centerIn: parent
                color: cdReady ? "#1a2a5a" : "#111122"
                border.color: cdReady ? "#4a6fc7" : "#2a2a4a"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: cdReady ? activeCd.toString() : "—"
                    color: cdReady ? "#7ab3ff" : "#3a3a6a"
                    font.pixelSize: 20
                    font.bold: true
                    font.family: "Courier"
                }
            }
        }

        // Track info — left side, mirrors BluetoothMusicView layout
        Column {
            anchors.left: parent.left
            anchors.leftMargin: 48
            anchors.right: discBox.left
            anchors.rightMargin: 32
            anchors.verticalCenter: parent.verticalCenter
            spacing: 20

            // CD number
            Column {
                width: parent.width
                spacing: 4
                Text { text: "CD"; color: "#555d72"; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1 }
                Text {
                    text: cdReady ? "Disc " + activeCd + " of 6" : "No Disc"
                    color: "white"
                    font.pixelSize: 26
                    font.bold: true
                    elide: Text.ElideRight
                    width: parent.width
                }
            }

            // Track
            Column {
                width: parent.width
                spacing: 4
                Text { text: "Track"; color: "#555d72"; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1 }
                Text {
                    text: cdReady
                          ? currentTrack + (totalTracks > 0 ? "  /  " + totalTracks : "")
                          : "—"
                    color: "#7ab3ff"
                    font.pixelSize: 26
                    font.bold: true
                    font.family: "Courier"
                    elide: Text.ElideRight
                    width: parent.width
                }
            }

            // Status
            Column {
                width: parent.width
                spacing: 4
                Text { text: "Status"; color: "#555d72"; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1 }
                Text {
                    text: !cdReady ? "No Disc Loaded"
                         : isPlaying ? "Playing" : "Paused"
                    color: "#aaaacc"
                    font.pixelSize: 18
                }
            }
        }
    }

    // ── Lower area ────────────────────────────────────────────────────────────
    Item {
        id: lowerArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: cdRow.top
        anchors.leftMargin: 32
        anchors.rightMargin: 32
        anchors.bottomMargin: 12
        height: 80

        // Progress bar
        Item {
            id: progressRow
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 24

            Text {
                id: timeLeft
                text: formatTime(currentTimeSec)
                color: "#666688"; font.pixelSize: 11
                anchors.left: parent.left
                anchors.verticalCenter: scrubTrack.verticalCenter
            }
            Text {
                id: timeRight
                text: formatTime(trackLengthSec)
                color: "#666688"; font.pixelSize: 11
                anchors.right: parent.right
                anchors.verticalCenter: scrubTrack.verticalCenter
            }

            Rectangle {
                id: scrubTrack
                anchors.left: timeLeft.right; anchors.leftMargin: 10
                anchors.right: timeRight.left; anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                height: 4; radius: 2
                color: "#2a2a3a"

                Rectangle {
                    id: scrubFill
                    width: Math.max(0, (scrubTrack.width - 14) * root.progress)
                    height: parent.height; radius: 2
                    color: "#7ab3ff"
                }

                Rectangle {
                    width: 14; height: 14; radius: 7
                    color: "white"
                    border.color: "#7ab3ff"; border.width: 2
                    anchors.verticalCenter: parent.verticalCenter
                    x: scrubFill.width
                }
            }
        }

        // Transport buttons — same order as BluetoothMusicView
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            spacing: 28

            // Previous track
            Rectangle {
                width: 48; height: 48; radius: 24
                color: prevArea.pressed ? "#2a2a4a" : "#1e2035"
                border.color: "#333355"; border.width: 1
                Text { text: "⏮"; font.pixelSize: 20; anchors.centerIn: parent }
                MouseArea {
                    id: prevArea; anchors.fill: parent
                    onClicked: uartController.send(0x12, 0x05)  // changeTrack=false, prev
                }
            }

            // Play / Pause
            Rectangle {
                width: 60; height: 60; radius: 30
                color: playArea.pressed ? "#5a8acc" : "#4a6fc7"
                border.color: "#6a8fd7"; border.width: 1
                Text {
                    text: isPlaying ? "⏸" : "▶"
                    font.pixelSize: 22
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: isPlaying ? 0 : 2
                }
                MouseArea {
                    id: playArea; anchors.fill: parent
                    // Toggle via CD state — the STM32 manages actual play/pause
                    onClicked: isPlaying = !isPlaying
                }
            }

            // Next track
            Rectangle {
                width: 48; height: 48; radius: 24
                color: nextArea.pressed ? "#2a2a4a" : "#1e2035"
                border.color: "#333355"; border.width: 1
                Text { text: "⏭"; font.pixelSize: 20; anchors.centerIn: parent }
                MouseArea {
                    id: nextArea; anchors.fill: parent
                    onClicked: uartController.send(0x12, 0x07)  // changeTrack=true, next
                }
            }
        }
    }

    // ── CD changer row ────────────────────────────────────────────────────────
    Row {
        id: cdRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        height: 56
        spacing: 8

        // ── Load button ───────────────────────────────────────────────────────
        Rectangle {
            width: 56; height: 56; radius: 10
            color: loadArea.pressed ? "#1a3a1a" : "#1a2a1a"
            border.color: "#2a5a2a"; border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 2
                Text { text: "⏏"; font.pixelSize: 16; color: "#4aaa4a"; anchors.horizontalCenter: parent.horizontalCenter; rotation: 180 }
                Text { text: "LOAD"; font.pixelSize: 9; color: "#4aaa4a"; font.letterSpacing: 1; anchors.horizontalCenter: parent.horizontalCenter }
            }
            MouseArea {
                id: loadArea; anchors.fill: parent
                // Send load command — disc number = activeCd
                onClicked: {
                    // CMD 0x12: changeCd=true(1), cdNumber=activeCd, changeTrack=false(0), trackPos=0
                    var data = Qt.btoa ? "" : ""   // placeholder — wire up sendData when ready
                    uartController.send(0x12, activeCd)
                }
            }
        }

        // ── CD 1–6 buttons ────────────────────────────────────────────────────
        Repeater {
            model: 6
            Rectangle {
                width: (cdRow.width - 56 - 56 - 7 * 8) / 6
                height: 56; radius: 10

                property bool isActive: (index + 1) === activeCd

                color:        isActive ? "#0a2a5a" : (cdBtn.pressed ? "#2a2a4a" : "#1e2035")
                border.color: isActive ? "#177cff" : "#2a2a4a"
                border.width: isActive ? 2 : 1

                Column {
                    anchors.centerIn: parent
                    spacing: 2

                    // Small disc icon
                    Rectangle {
                        width: 18; height: 18; radius: 9
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: isActive ? "#1a3a6a" : "#1a1a2e"
                        border.color: isActive ? "#4a8aff" : "#333355"
                        border.width: 1
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            anchors.centerIn: parent
                            color: isActive ? "#7ab3ff" : "#2a2a4a"
                        }
                    }

                    Text {
                        text: (index + 1).toString()
                        color: isActive ? "#7ab3ff" : "#666688"
                        font.pixelSize: 14
                        font.bold: isActive
                        font.family: "Courier"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                MouseArea {
                    id: cdBtn; anchors.fill: parent
                    onClicked: {
                        activeCd = index + 1
                        // CMD 0x12: changeCd=true, cdNumber, changeTrack=false, trackPos=0
                        uartController.send(0x12, activeCd)
                    }
                }
            }
        }

        // ── Eject button ──────────────────────────────────────────────────────
        Rectangle {
            width: 56; height: 56; radius: 10
            color: ejectArea.pressed ? "#3a1a1a" : "#2a1a1a"
            border.color: "#5a2a2a"; border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 2
                Text { text: "⏏"; font.pixelSize: 16; color: "#aa4a4a"; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: "EJECT"; font.pixelSize: 9; color: "#aa4a4a"; font.letterSpacing: 1; anchors.horizontalCenter: parent.horizontalCenter }
            }
            MouseArea {
                id: ejectArea; anchors.fill: parent
                onClicked: {
                    cdReady  = false
                    isPlaying = false
                    uartController.send(0x12, 0x00)  // eject current disc
                }
            }
        }
    }
}
