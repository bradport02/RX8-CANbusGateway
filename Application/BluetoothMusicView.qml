import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "transparent"

    function formatTime(ms) {
        var s = Math.floor(ms / 1000)
        var m = Math.floor(s / 60)
        s = s % 60
        return m + ":" + (s < 10 ? "0" : "") + s
    }

    property bool isPlaying: mediaPlayer.status === "playing"
    property real progress:  mediaPlayer.duration > 0
                             ? Math.min(mediaPlayer.position / mediaPlayer.duration, 1.0)
                             : 0.0

    // ── No device connected ───────────────────────────────────────────────────
    Column {
        anchors.centerIn: parent
        spacing: 12
        visible: !bluetoothManager.connected

        Text { text: "🎵"; font.pixelSize: 48; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: "No device connected"; color: "#666666"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
    }

    // ── Main layout ───────────────────────────────────────────────────────────
    Item {
        anchors.fill: parent
        visible: bluetoothManager.connected

        // ── Upper half: track info + artwork ─────────────────────────────────
        Item {
            id: upperArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: lowerArea.top

            // Artwork — right side, centred vertically
            Rectangle {
                id: artworkBox
                width: 180; height: 180
                anchors.right: parent.right
                anchors.rightMargin: 48
                anchors.verticalCenter: parent.verticalCenter
                radius: 14
                color: "#22223a"
                border.color: "#3a3a5a"
                border.width: 1

                Image {
                    id: artImg
                    anchors.fill: parent
                    source: mediaPlayer.artworkPath ? "file://" + mediaPlayer.artworkPath : ""
                    fillMode: Image.PreserveAspectCrop
                    visible: status === Image.Ready
                }

                Text {
                    anchors.centerIn: parent
                    text: "♪"
                    color: "#3a3a7a"
                    font.pixelSize: 90
                    visible: !artImg.visible

                    SequentialAnimation on opacity {
                        running: root.isPlaying
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 1400; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 1.0; duration: 1400; easing.type: Easing.InOutSine }
                    }
                    opacity: root.isPlaying ? 1.0 : 0.25
                }
            }

            // Track info — left side, centred vertically
            Column {
                anchors.left: parent.left
                anchors.leftMargin: 48
                anchors.right: artworkBox.left
                anchors.rightMargin: 32
                anchors.verticalCenter: parent.verticalCenter
                spacing: 20

                // Artist
                Column {
                    width: parent.width
                    spacing: 4
                    Text { text: "Artist"; color: "#555d72"; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1 }
                    Text {
                        text: mediaPlayer.artist || "—"
                        color: "white"
                        font.pixelSize: 26
                        font.bold: true
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }

                // Album
                Column {
                    width: parent.width
                    spacing: 4
                    Text { text: "Album"; color: "#555d72"; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1 }
                    Text {
                        text: mediaPlayer.album || "—"
                        color: "#aaaacc"
                        font.pixelSize: 18
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }

                // Song
                Column {
                    width: parent.width
                    spacing: 4
                    Text { text: "Song"; color: "#555d72"; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1 }
                    Text {
                        text: mediaPlayer.title || "—"
                        color: "#7ab3ff"
                        font.pixelSize: 18
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }
            }
        }

        // ── Lower area: progress + buttons — fixed height at bottom ───────────
        Item {
            id: lowerArea
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 32
            anchors.rightMargin: 32
            anchors.bottomMargin: 24
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
                    text: formatTime(mediaPlayer.position)
                    color: "#666688"; font.pixelSize: 11
                    anchors.left: parent.left
                    anchors.verticalCenter: scrubTrack.verticalCenter
                }
                Text {
                    id: timeRight
                    text: formatTime(mediaPlayer.duration)
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

            // Transport buttons
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                spacing: 28

                Rectangle {
                    width: 48; height: 48; radius: 24
                    color: prevArea.pressed ? "#2a2a4a" : "#1e2035"
                    border.color: "#333355"; border.width: 1
                    Text { text: "⏮"; font.pixelSize: 20; anchors.centerIn: parent }
                    MouseArea { id: prevArea; anchors.fill: parent; onClicked: mediaPlayer.previous() }
                }

                Rectangle {
                    width: 60; height: 60; radius: 30
                    color: playArea.pressed ? "#5a8acc" : "#4a6fc7"
                    border.color: "#6a8fd7"; border.width: 1
                    Text {
                        text: root.isPlaying ? "⏸" : "▶"
                        font.pixelSize: 22
                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: root.isPlaying ? 0 : 2
                    }
                    MouseArea { id: playArea; anchors.fill: parent; onClicked: root.isPlaying ? mediaPlayer.pause() : mediaPlayer.play() }
                }

                Rectangle {
                    width: 48; height: 48; radius: 24
                    color: nextArea.pressed ? "#2a2a4a" : "#1e2035"
                    border.color: "#333355"; border.width: 1
                    Text { text: "⏭"; font.pixelSize: 20; anchors.centerIn: parent }
                    MouseArea { id: nextArea; anchors.fill: parent; onClicked: mediaPlayer.next() }
                }
            }
        }
    }
}
