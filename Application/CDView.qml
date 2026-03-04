import QtQuick

Rectangle {
    color: "#111118"

    Column {
        anchors.centerIn: parent
        spacing: 20

        // Spinning disc
        Item {
            width: 160; height: 160
            anchors.horizontalCenter: parent.horizontalCenter

            Rectangle {
                anchors.fill: parent; radius: 80
                color: "#1e1e2e"
                border.color: "#2a2a3a"; border.width: 1

                // CD rainbow shimmer
                Rectangle {
                    anchors.fill: parent; radius: 80
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#44ff6b6b" }
                        GradientStop { position: 0.3; color: "#446bffaa" }
                        GradientStop { position: 0.6; color: "#446baeff" }
                        GradientStop { position: 1.0; color: "#44ff6b6b" }
                    }
                }

                // Centre hole
                Rectangle {
                    width: 30; height: 30; radius: 15
                    anchors.centerIn: parent
                    color: "#111118"
                    border.color: "#2a2a3a"; border.width: 1
                }
            }

            Text {
                anchors.centerIn: parent
                text: "💿"; font.pixelSize: 48
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "CD Player"
            color: "white"
            font.pixelSize: 22
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "No disc inserted"
            color: "#555577"
            font.pixelSize: 14
        }

        // Eject button
        Rectangle {
            width: 120; height: 40; radius: 10
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#1e1e2e"
            border.color: "#2a2a3a"; border.width: 1
            Row {
                anchors.centerIn: parent; spacing: 8
                Text { text: "⏏"; color: "#7ab3ff"; font.pixelSize: 18; anchors.verticalCenter: parent.verticalCenter }
                Text { text: "Eject"; color: "#7ab3ff"; font.pixelSize: 14; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
            }
            MouseArea { anchors.fill: parent }
        }
    }
}
