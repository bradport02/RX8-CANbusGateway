import QtQuick
import QtQuick.Controls

Rectangle {
    id: mediaPage
    color: "#111118"

    // ── Active source ─────────────────────────────────────────────────────────
    property int activeSource: 0   // 0=Radio, 1=Bluetooth, 2=AUX, 3=CD, 4=Carplay

    property string activeSourceText: "Radio"

    readonly property var sources: [
        { name: "Radio",     icon: "📻" },
        { name: "Bluetooth", icon: "🎵" },
        { name: "AUX",       icon: "🔌" },
        { name: "CD",        icon: "💿" }
    ]

    // ── Source selector bar ───────────────────────────────────────────────────
    Rectangle {
        id: sourceBar
        width: parent.width
        height: 64
        color: "#1a1a22"
        z: 10

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "#2a2a3a"
        }

        Row {
            anchors.fill: parent
            spacing: 0

            Repeater {
                model: sources

                delegate: Item {
                    width: mediaPage.width / sources.length
                    height: sourceBar.height

                    // Active indicator bar at bottom
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: "#7ab3ff"
                        visible: mediaPage.activeSource === index
                        Behavior on opacity { NumberAnimation { duration: 200 } }
                    }

                    // Background highlight
                    Rectangle {
                        anchors.fill: parent
                        color: mediaPage.activeSource === index ? "#1e2035" : "transparent"
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 4

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.icon
                            font.pixelSize: 20
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.name
                            color: mediaPage.activeSource === index ? "#7ab3ff" : "#666688"
                            font.pixelSize: 10
                            font.bold: mediaPage.activeSource === index
                            font.letterSpacing: 1
                        }
                    }

                    function sourceView() {
                        return activeSource === 0 ? "Radio"
                             : activeSource === 1 ? "BT Music"
                             : activeSource === 2 ? "Aux Input"
                             : "CD Player"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            mediaPage.activeSource = index
                            uartController.sendLCDText(sourceView())
                            uartController.send(0x16,activeSource)
                        }
                    }
                }
            }
        }
    }

    // ── Source content area ───────────────────────────────────────────────────
    RadioSourceView {
        anchors.top: sourceBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: mediaPage.activeSource === 0
    }

    BluetoothMusicView {
        anchors.top: sourceBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: mediaPage.activeSource === 1
    }

    AuxView {
        anchors.top: sourceBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: mediaPage.activeSource === 2
    }

    CDView {
        anchors.top: sourceBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: mediaPage.activeSource === 3
    }
}
