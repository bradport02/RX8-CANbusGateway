import QtQuick
import QtQuick.Controls

Rectangle {
    color: "#1a1a1a"

    Component.onCompleted: {
        uartController.sendLCDText("Main Menu")
    }

    Grid {
        anchors.verticalCenterOffset: 100
        anchors.horizontalCenterOffset: 0
        rotation: 90
        anchors.centerIn: parent
        columns: 1
        spacing: 20

        Button {
            text: "Settings"
            width: 100
            height: 100

            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                rotation: -90
            }

            background: Rectangle {
                color: "#4a4a4a"
                radius: width/2
            }

            onClicked: stackView.push(settingsPage)
        }

        Button {
            text: "Climate"
            width: 100
            height: 100

            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                rotation: -90
            }

            background: Rectangle {
                color: "#4a4a4a"
                radius: width/2
            }

            onClicked: stackView.push(climatePage)
        }

        Button {
            text: "Phone"
            width: 100
            height: 100

            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                rotation: -90
            }

            background: Rectangle {
                color: "#4a4a4a"
                radius: width/2
            }

            onClicked: stackView.push(bluetoothPage)
        }

        Button {
            text: "Media"
            width: 100
            height: 100

            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                rotation: -90
            }

            background: Rectangle {
                color: "#4a4a4a"
                radius: width/2
            }

            onClicked: stackView.push(mediaPage)
        }

        Button {
            text: "Carplay"
            width: 100
            height: 100

            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                rotation: -90
            }

            background: Rectangle {
                color: "#4a4a4a"
                radius: width/2
            }

            onClicked: stackView.push(carplayPage)
        }
    }
}
