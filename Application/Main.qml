import QtQuick
import QtQuick.Layouts 2.15
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    color: "#1a1a1a"
    title: qsTr("Application")

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
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2a2a2a"

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

                    // Reactively bound — updates instantly when toggled in settings
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

                // Signal strength bars
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

                // Carrier name
                Text {
                    visible: bluetoothManager ? (bluetoothManager.connected && bluetoothManager.networkOperator !== "") : false
                    anchors.verticalCenter: parent.verticalCenter
                    text: (bluetoothManager && bluetoothManager.networkRoaming)
                          ? bluetoothManager.networkOperator + " ⟳"
                          : (bluetoothManager ? bluetoothManager.networkOperator : "")
                    color: "#c2c2c2"
                    font.pixelSize: 25
                }

                // Divider
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
}
