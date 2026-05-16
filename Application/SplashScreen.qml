import QtQuick

Rectangle {
    color: "black"

    Image {
        id: logo
        anchors.centerIn: parent
        source: "qrc:/assets/rhul_logo.png"
        // adjust width/height to suit your logo
    }

    Text {
        id: errorText
        anchors.top: logo.bottom
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#cc0000"
        font.pixelSize: 18
        visible: false
        text: "Something went wrong — STM32 no comms\nBooting without full functionality."
        horizontalAlignment: Text.AlignHCenter
    }

    Timer {
        id: handshakeTimeout
        interval: 5000
        running: !uartController.handshakeComplete
        onTriggered: {
            if (!uartController.handshakeComplete) {
                errorText.visible = true
                failsafeTimer.start()
            }
        }
    }

    Timer {
        id: failsafeTimer
        interval: 3000
        onTriggered: rootLoader.sourceComponent = mainApp
    }

    Connections {
        target: uartController
        function onHandshakeCompleteChanged() {
            if (uartController.handshakeComplete) {
                handshakeTimeout.stop()
                rootLoader.sourceComponent = mainApp
            }
        }
    }
}
