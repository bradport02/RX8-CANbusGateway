import QtQuick

Rectangle {
    color: "#111118"

    Component.onCompleted: {
        uartController.sendLCDText("Aux Input")
    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text { anchors.horizontalCenter: parent.horizontalCenter; text: "🔌"; font.pixelSize: 72 }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "AUX Input"
            color: "white"
            font.pixelSize: 22
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Connect a device to the AUX port"
            color: "#555577"
            font.pixelSize: 14
        }

        // Input level indicator
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "INPUT LEVEL"
                color: "#555d72"
                font.pixelSize: 9
                font.bold: true
                font.letterSpacing: 2.5
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 3

                Repeater {
                    model: 20
                    Rectangle {
                        width: 14; height: 28; radius: 3
                        color: index < 14 ? "#4a6fc7"
                             : index < 17 ? "#ffaa44"
                             : "#ff4444"
                        opacity: 0.3  // static placeholder — wire to real level meter
                    }
                }
            }
        }
    }
}
