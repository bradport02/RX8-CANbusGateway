import QtQuick
import QtQuick.Layouts 2.15
import QtQuick.Controls
import Qt.labs.settings 1.0

Window {
    id: root
    width: 640
    height: 480
    visible: true
    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    color: "#1a1a1a"
    title: qsTr("Application")

    signal volumeChangeRequested(int volume)

    Connections {
        target: callManager
        function onCallStateChanged() {
            var state = callManager.callState
            if (state === 1 || state === 2 || state === 3) {
                if (stackView.currentItem.toString().indexOf("ActiveCallPage") === -1)
                    stackView.push(activeCallPage)
            } else if (state === 0) {
                if (stackView.currentItem.toString().indexOf("ActiveCallPage") !== -1)
                    stackView.pop()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Top bar ───────────────────────────────────────────────────────────
        Rectangle {
            id: topBar
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2a2a2a"
            z: 100
            clip: false

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                // Home button
                Rectangle {
                    width: 50; height: 50
                    color: "#c2c2c2"; radius: 5
                    anchors.verticalCenter: parent.verticalCenter
                    Text { text: "⌂"; font.pixelSize: 30; anchors.centerIn: parent }
                    MouseArea { anchors.fill: parent; onClicked: stackView.pop(null) }
                }

                Rectangle {
                    width: 2; height: 30; color: "#c2c2c2"
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Back button
                Rectangle {
                    width: 50; height: 50
                    color: "#c2c2c2"; radius: 5
                    anchors.verticalCenter: parent.verticalCenter
                    Text { text: "↩"; font.pixelSize: 30; anchors.centerIn: parent }
                    MouseArea { anchors.fill: parent; onClicked: stackView.pop() }
                }

                Rectangle {
                    width: 2; height: 30; color: "#c2c2c2"
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Volume button
                Rectangle {
                    id: volumeBtn
                    width: 50; height: 50
                    color: volumePopup.opened ? "#177cff" : "#c2c2c2"; radius: 5
                    anchors.verticalCenter: parent.verticalCenter
                    Text { text: "🔊"; font.pixelSize: 30; anchors.centerIn: parent }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: volumePopup.opened = !volumePopup.opened
                    }
                }
            }

            // ── Centre: date | time ───────────────────────────────────────────
            Row {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    id: dateText
                    color: "#c2c2c2"
                    font.pixelSize: 25
                    anchors.verticalCenter: parent.verticalCenter
                    Component.onCompleted: text = Qt.formatDate(new Date(), "ddd d MMM")
                }

                Rectangle {
                    width: 2; height: 30; color: "#c2c2c2"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: clockText
                    color: "#c2c2c2"
                    font.pixelSize: 25
                    anchors.verticalCenter: parent.verticalCenter

                    property bool use24h: systemClock ? systemClock.use24h : true

                    function fmtTime(d) {
                        return use24h
                            ? Qt.formatTime(d, "HH:mm")
                            : Qt.formatTime(d, "hh:mm AP")
                    }

                    onUse24hChanged: text = fmtTime(new Date())

                    Timer {
                        interval: 1000; running: true; repeat: true
                        onTriggered: {
                            var d = new Date()
                            clockText.text = clockText.fmtTime(d)
                            dateText.text  = Qt.formatDate(d, "ddd d MMM")
                        }
                    }
                    Component.onCompleted: text = fmtTime(new Date())
                }
            }

            // ── Right: signal bars + carrier ──────────────────────────────────
            Row {
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Row {
                    id: signalBars
                    spacing: 4
                    anchors.verticalCenter: parent.verticalCenter
                    visible: bluetoothManager ? bluetoothManager.connected : false

                    Repeater {
                        model: 5
                        Item {
                            width: 6; height: 26
                            anchors.verticalCenter: parent.verticalCenter
                            Rectangle {
                                width: parent.width
                                height: 10 + index * 5
                                radius: 2
                                anchors.bottom: parent.bottom
                                color: (bluetoothManager && index < bluetoothManager.networkStrength)
                                       ? "#177cff" : "#c2c2c2"
                            }
                        }
                    }
                }

                Text {
                    visible: bluetoothManager ? (bluetoothManager.connected && bluetoothManager.networkOperator !== "") : false
                    anchors.verticalCenter: parent.verticalCenter
                    text: (bluetoothManager && bluetoothManager.networkRoaming)
                          ? bluetoothManager.networkOperator + " ⟳"
                          : (bluetoothManager ? bluetoothManager.networkOperator : "")
                    color: "#c2c2c2"
                    font.pixelSize: 25
                }

                Rectangle {
                    width: 2; height: 30; color: "#c2c2c2"
                    anchors.verticalCenter: parent.verticalCenter
                    visible: bluetoothManager ? bluetoothManager.connected : false
                }
            }


        }

        // ── Main content ──────────────────────────────────────────────────────
        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: homePage

            // Close volume popup when navigating
            onCurrentItemChanged: volumePopup.opened = false



            pushEnter: Transition {
                ParallelAnimation {
                    PropertyAnimation { property: "scale";   from: 0.8; to: 1;   duration: 300; easing.type: Easing.OutCubic }
                    PropertyAnimation { property: "opacity"; from: 0;   to: 1;   duration: 300 }
                }
            }
            pushExit: Transition {
                PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 300 }
            }
            popEnter: Transition {
                PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 300 }
            }
            popExit: Transition {
                ParallelAnimation {
                    PropertyAnimation { property: "scale";   from: 1;   to: 0.8; duration: 300; easing.type: Easing.OutCubic }
                    PropertyAnimation { property: "opacity"; from: 1;   to: 0;   duration: 300 }
                }
            }

            Component { id: homePage;               HomePage                {}  }
            Component { id: mediaPage;              MediaPage               {}  }
            Component { id: carplayPage;            CarplayPage             {}  }
            Component { id: bluetoothPage;          PhonePage               {}  }
            Component { id: activeCallPage;         ActiveCallPage          {}  }
            Component { id: settingsPage;           SettingsPage            {}  }
            Component { id: lightingPage;           AmbientLightingPage     {}  }
            Component { id: clockSettingsPage;      ClockSettingsPage       {}  }
            Component { id: bluetoothSettingsPage;  BluetoothSettingsPage   {}  }
            Component { id: canbusPage;             CANBusPage              {}  }
            Component { id: canRawDataPage;         CANRawDataPage          {}  }
            Component { id: climatePage;            ClimatePage             {}  }
        }
    }
    // ── Dismiss by tapping outside popup ─────────────────────────────────────
    MouseArea {
        anchors.fill: parent
        enabled: volumePopup.opened
        z: 998  // below popup (z:999) so slider still receives events
        onClicked: volumePopup.opened = false
    }

    // ── Volume popup — direct Window child for correct input handling ────────
    Rectangle {
        id: volumePopup
        property bool opened: false
        visible: opened
        width: 260
        height: 90
        radius: 10
        color: "#1e1e2e"
        border.color: "#177cff"
        border.width: 1
        z: 999
        // Position below volume button (home50 + sep2 + back50 + sep2 + vol50 = x:148)
        x: 148
        y: 66

        // Triangle pointer
        Canvas {
            width: 16; height: 10
            x: 17; y: -10
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.fillStyle = "#177cff"
                ctx.beginPath()
                ctx.moveTo(0, 10); ctx.lineTo(8, 0); ctx.lineTo(16, 10)
                ctx.closePath(); ctx.fill()
                ctx.fillStyle = "#1e1e2e"
                ctx.beginPath()
                ctx.moveTo(1, 10); ctx.lineTo(8, 1); ctx.lineTo(15, 10)
                ctx.closePath(); ctx.fill()
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 8

            Row {
                width: parent.width
                Text {
                    text: "🔊  VOLUME"
                    color: "#888899"
                    font.pixelSize: 11
                    font.letterSpacing: 2
                    font.capitalization: Font.AllUppercase
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: parent.width - volLabel.implicitWidth - 100; height: 1 }
                Text {
                    id: volLabel
                    text: volumeSliderItem.volume
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }
            }

            Item {
                id: volumeSliderItem
                width: parent.width
                height: 28
                property int volume: 15

                Rectangle {
                    width: parent.width; height: 6; radius: 3
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#2a2a4a"
                    Rectangle {
                        width: (volumeSliderItem.volume / 30) * parent.width
                        height: parent.height; radius: parent.radius
                        color: "#177cff"
                    }
                }

                Rectangle {
                    width: 24; height: 24; radius: 12
                    color: "white"
                    border.color: "#177cff"; border.width: 2
                    anchors.verticalCenter: parent.verticalCenter
                    x: (volumeSliderItem.volume / 30) * (volumeSliderItem.width - width)
                }

                MouseArea {
                    anchors.fill: parent
                    preventStealing: true
                    onPressed:         updateVol(mouseX)
                    onPositionChanged: updateVol(mouseX)
                    function updateVol(mx) {
                        volumeSliderItem.volume = Math.round(
                            (Math.max(0, Math.min(mx, volumeSliderItem.width))
                            / volumeSliderItem.width) * 30)
                        root.volumeChangeRequested(volumeSliderItem.volume)
                    }
                }
            }
        }
    }
}
