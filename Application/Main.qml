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
                // Push call page if not already showing it
                if (stackView.currentItem.toString().indexOf("ActiveCallPage") === -1)
                    stackView.push(activeCallPage)
            } else if (state === 0) {
                // Pop back when call ends
                if (stackView.currentItem.toString().indexOf("ActiveCallPage") !== -1)
                    stackView.pop()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Top bar - always visible
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#2a2a2a"

            // Home button in top left
            Rectangle {
                width: 50
                height: 50
                color: "#3a3a3a"
                radius: 5
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 10

                Text {
                    text: "↩"
                    font.pixelSize: 30
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: stackView.pop()
                }
            }

            Text {
                text: "Car Headunit"
                color: "white"
                font.pixelSize: 24
                anchors.centerIn: parent
            }

            // Clock in top right corner
            Text {
                id: clockText
                color: "white"
                font.pixelSize: 20
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 20

                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: {
                        var date = new Date()
                        clockText.text = Qt.formatTime(date, "hh:mm")
                    }
                }

                Component.onCompleted: {
                    var date = new Date()
                    text = Qt.formatTime(date, "hh:mm")
                }
            }
        }

        // Main content area with scale/zoom animations
        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: homePage

            // Scale/Zoom animation when pushing
            pushEnter: Transition {
                ParallelAnimation {
                    PropertyAnimation {
                        property: "scale"
                        from: 0.8
                        to: 1
                        duration: 300
                        easing.type: Easing.OutCubic
                    }
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 300
                    }
                }
            }

            pushExit: Transition {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 300
                }
            }

            popEnter: Transition {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 300
                }
            }

            popExit: Transition {
                ParallelAnimation {
                    PropertyAnimation {
                        property: "scale"
                        from: 1
                        to: 0.8
                        duration: 300
                        easing.type: Easing.OutCubic
                    }
                    PropertyAnimation {
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 300
                    }
                }
            }

            Component {
                id: homePage
                HomePage {}
            }

            Component {
                id: radioPage
                RadioPage {}
            }

            Component {
                id: carplayPage
                CarplayPage {}
            }

            Component {
                id: bluetoothPage
                PhonePage {}
            }

            Component {
                id: activeCallPage
                ActiveCallPage{}
            }

            Component {
                id: settingsPage
                SettingsPage {}
            }

            Component {
                id: lightingPage
                AmbientLightingPage {}
            }

            Component{
                id: bluetoothSettingsPage
                BluetoothSettingsPage{}
            }

            Component {
                id: canbusPage
                CANBusPage {}
            }

            Component {
                id: canRawDataPage
                CANRawDataPage {}
            }

            Component {
                id: climatePage
                ClimatePage {}
            }
        }
    }
}
