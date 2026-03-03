import QtQuick
import QtQuick.Controls

Rectangle {
    id: carplayPage
    color: "#1a1a1a"

    // Launch CarPlay when this page becomes visible
    Component.onCompleted: {
        carplayController.launchCarPlay()
    }

    Component.onDestruction: {
        carplayController.stopCarPlay()
    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "CarPlay Active"
            color: "white"
            font.pixelSize: 32
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Connect your iPhone via USB"
            color: "#888888"
            font.pixelSize: 16
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Manual exit button (backup)
        Button {
            text: "Exit CarPlay"
            width: 200
            height: 60
            anchors.horizontalCenter: parent.horizontalCenter

            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: "#ff4444"
                radius: 10
            }

            onClicked: {
                carplayController.stopCarPlay()
                stackView.pop()  // Go back to home
            }
        }
    }
}
