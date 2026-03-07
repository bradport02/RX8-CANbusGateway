import QtQuick
import QtQuick.Controls
import Qt.labs.settings 1.0

Item {
    id: root

    Settings {
        id: settings
        category: "audio"
        property int bass:    0   // -5 to +5
        property int mids:    0   // -5 to +5
        property int treble:  0   // -5 to +5
        property int balance: 0   // -5 (left) to +5 (right)
        property int fader:   0   // -5 (rear) to +5 (front)
    }
    // Import from settings
    property int bass:    settings.bass   // -5 to +5
    property int mids:    settings.mids   // -5 to +5
    property int treble:  settings.treble   // -5 to +5
    property int balance: settings.balance  // -5 (left) to +5 (right)
    property int fader:   settings.fader   // -5 (rear) to +5 (front)

    // Persist on every change
    onBassChanged:      settings.bass       = bass
    onMidsChanged:      settings.mids       = mids
    onTrebleChanged:    settings.treble     = treble
    onBalanceChanged:   settings.balance    = balance
    onFaderChanged:     settings.fader      = fader

    // ── Helpers ───────────────────────────────────────────────────────────────
    function signedLabel(v) { return v > 0 ? "+" + v : "" + v }

    // ── Background ────────────────────────────────────────────────────────────
    Rectangle { anchors.fill: parent; color: "#0e0e16" }

    // ════════════════════════════════════════════════════════════════════════
    // LAYOUT  — left panel (EQ) | divider | right panel (spatial pad)
    // ════════════════════════════════════════════════════════════════════════
    Row {
        anchors.fill: parent
        anchors.margins: 0

        // ── LEFT: EQ sliders ─────────────────────────────────────────────────
        Item {
            width: parent.width * 0.42
            height: parent.height

            Column {
                anchors.centerIn: parent
                spacing: 28

                // Section label
                Text {
                    text: "EQUALISER"
                    color: "#2a5aaa"
                    font.pixelSize: 10
                    font.letterSpacing: 4
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // EQ band component
                component EQBand: Column {
                    id: band
                    property string label: "BASS"
                    property int    value: 0   // -5 to +5
                    property color  accent: "#177cff"
                    spacing: 6

                    signal changed(int v)

                    // Value readout + label
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 10

                        Text {
                            text: band.label
                            color: "#555577"
                            font.pixelSize: 10
                            font.letterSpacing: 3
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Rectangle {
                            width: 46; height: 28; radius: 6
                            color: "#16162a"
                            border.color: band.accent
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                anchors.centerIn: parent
                                text: signedLabel(band.value)
                                color: band.value === 0 ? "#888899" : band.accent
                                font.pixelSize: 14
                                font.bold: true
                                font.family: "Courier"
                            }
                        }
                    }

                    // Vertical slider track
                    Item {
                        id: eqTrackArea
                        width: 44
                        height: 160
                        anchors.horizontalCenter: parent.horizontalCenter

                        // Centre line
                        Rectangle {
                            width: parent.width; height: 1
                            anchors.verticalCenter: parent.verticalCenter
                            color: "#222233"
                        }

                        // Track
                        Rectangle {
                            id: eqTrack
                            width: 6
                            height: parent.height
                            radius: 3
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: "#1a1a2e"

                            // Fill from centre to handle
                            Rectangle {
                                width: parent.width
                                radius: parent.radius
                                color: band.accent
                                opacity: 0.8

                                // Centre of track in pixels
                                property real centre: eqTrack.height / 2
                                property real handleY: ((5 - band.value) / 10) * eqTrack.height

                                y:      band.value >= 0 ? handleY : centre
                                height: Math.abs(centre - handleY)
                            }
                        }

                        // Tick marks at -5, 0, +5
                        Repeater {
                            model: [0, 0.5, 1.0]
                            Rectangle {
                                width: 12; height: 1
                                color: "#2a2a44"
                                x: (eqTrackArea.width - width) / 2
                                y: modelData * (eqTrackArea.height - 1)
                            }
                        }

                        // Handle
                        Rectangle {
                            id: eqHandle
                            width: 22; height: 22; radius: 11
                            color: "#0e0e16"
                            border.color: band.accent
                            border.width: 2
                            anchors.horizontalCenter: parent.horizontalCenter
                            y: ((5 - band.value) / 10) * (eqTrackArea.height - height)

                            // Glow ring
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width + 8; height: parent.height + 8
                                radius: width / 2
                                color: "transparent"
                                border.color: band.accent
                                border.width: 1
                                opacity: 0.3
                                z: -1
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            preventStealing: true
                            function updateFromY(my) {
                                var clamped = Math.max(0, Math.min(my, eqTrackArea.height))
                                var v = Math.round(5 - (clamped / eqTrackArea.height) * 10)
                                band.value = Math.max(-5, Math.min(5, v))
                                band.changed(band.value)
                            }
                            onPressed:         updateFromY(mouseY)
                            onPositionChanged: updateFromY(mouseY)
                        }
                    }
                }

                // Three EQ bands side by side
                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    EQBand {
                        label: "BASS"
                        value: bass
                        accent: "#ff6b35"
                        onChanged: function(v) { bass = v }
                    }
                    EQBand {
                        label: "MIDS"
                        value: mids
                        accent: "#177cff"
                        onChanged: function(v) { mids = v }
                    }
                    EQBand {
                        label: "TREBLE"
                        value: treble
                        accent: "#2aff9a"
                        onChanged: function(v) { treble = v }
                    }
                }

                // Reset button
                Rectangle {
                    width: 100; height: 32; radius: 8
                    color: "#16162a"
                    border.color: "#2a2a44"; border.width: 1
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "RESET EQ"
                        color: "#444466"
                        font.pixelSize: 10
                        font.letterSpacing: 2
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { bass = 0; mids = 0; treble = 0 }
                    }
                }
            }
        }

        // ── DIVIDER ───────────────────────────────────────────────────────────
        Rectangle {
            width: 1
            height: parent.height
            color: "#1a1a2e"
        }

        // ── RIGHT: Spatial pad (balance + fader) ──────────────────────────────
        Item {
            width: parent.width - parent.width * 0.42 - 1
            height: parent.height

            Column {
                anchors.centerIn: parent
                spacing: 16

                Text {
                    text: "SPATIAL"
                    color: "#2a5aaa"
                    font.pixelSize: 10
                    font.letterSpacing: 4
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // ── 2D pad ────────────────────────────────────────────────────
                Item {
                    id: spatialPad
                    width: 240
                    height: 240
                    anchors.horizontalCenter: parent.horizontalCenter

                    // Quadrant background shading
                    Repeater {
                        model: [
                            { qx: 0,   qy: 0,   label: "FL" },
                            { qx: 0.5, qy: 0,   label: "FR" },
                            { qx: 0,   qy: 0.5, label: "RL" },
                            { qx: 0.5, qy: 0.5, label: "RR" }
                        ]
                        Rectangle {
                            x: modelData.qx * spatialPad.width
                            y: modelData.qy * spatialPad.height
                            width: spatialPad.width / 2
                            height: spatialPad.height / 2
                            color: "#0a0a14"
                            border.color: "#1a1a2e"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: modelData.label
                                color: "#1e1e38"
                                font.pixelSize: 22
                                font.bold: true
                                font.family: "Courier"
                            }
                        }
                    }

                    // Centre crosshair
                    Rectangle {
                        width: spatialPad.width; height: 1
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#2a2a44"
                    }
                    Rectangle {
                        width: 1; height: spatialPad.height
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "#2a2a44"
                    }

                    // Axis labels
                    Text { text: "FRONT"; color: "#2a2a55"; font.pixelSize: 9; font.letterSpacing: 2
                           anchors.horizontalCenter: parent.horizontalCenter; y: 4 }
                    Text { text: "REAR";  color: "#2a2a55"; font.pixelSize: 9; font.letterSpacing: 2
                           anchors.horizontalCenter: parent.horizontalCenter; y: spatialPad.height - 16 }
                    Text { text: "L"; color: "#2a2a55"; font.pixelSize: 9
                           x: 4; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "R"; color: "#2a2a55"; font.pixelSize: 9
                           x: spatialPad.width - 12; anchors.verticalCenter: parent.verticalCenter }

                    // Puck — position driven by balance/fader values
                    Rectangle {
                        id: puck
                        width: 28; height: 28; radius: 14
                        color: "#0e0e16"
                        border.color: "#177cff"
                        border.width: 2
                        z: 10

                        // balance: -5(left) to +5(right) → x: 0 to pad.width
                        x: ((balance + 5) / 10) * (spatialPad.width  - width)
                        // fader:   +5(front) to -5(rear) → y: 0(top=front) to pad.height
                        y: ((5 - fader)    / 10) * (spatialPad.height - height)

                        // Inner dot
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: "#177cff"
                            anchors.centerIn: parent
                        }

                        // Pulse ring
                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width + 10; height: parent.height + 10
                            radius: width / 2
                            color: "transparent"
                            border.color: "#177cff"
                            border.width: 1
                            opacity: 0.25
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        preventStealing: true

                        function updateFromPos(mx, my) {
                            var cx = Math.max(0, Math.min(mx, spatialPad.width))
                            var cy = Math.max(0, Math.min(my, spatialPad.height))
                            balance = Math.round(((cx / spatialPad.width)  * 10) - 5)
                            fader   = Math.round(5 - ((cy / spatialPad.height) * 10))
                        }
                        onPressed:         updateFromPos(mouseX, mouseY)
                        onPositionChanged: updateFromPos(mouseX, mouseY)
                    }
                }

                // Value readouts
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 24

                    Column {
                        spacing: 4
                        Text {
                            text: "BALANCE"
                            color: "#444466"; font.pixelSize: 9
                            font.letterSpacing: 2; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Rectangle {
                            width: 60; height: 28; radius: 6
                            color: "#16162a"; border.color: "#177cff"; border.width: 1
                            anchors.horizontalCenter: parent.horizontalCenter
                            Text {
                                anchors.centerIn: parent
                                text: signedLabel(balance)
                                color: balance === 0 ? "#888899" : "#177cff"
                                font.pixelSize: 14; font.bold: true; font.family: "Courier"
                            }
                        }
                        Text {
                            text: balance < 0 ? "LEFT" : balance > 0 ? "RIGHT" : "CENTRE"
                            color: "#333355"; font.pixelSize: 8; font.letterSpacing: 1
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    Column {
                        spacing: 4
                        Text {
                            text: "FADER"
                            color: "#444466"; font.pixelSize: 9
                            font.letterSpacing: 2; font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Rectangle {
                            width: 60; height: 28; radius: 6
                            color: "#16162a"; border.color: "#177cff"; border.width: 1
                            anchors.horizontalCenter: parent.horizontalCenter
                            Text {
                                anchors.centerIn: parent
                                text: signedLabel(fader)
                                color: fader === 0 ? "#888899" : "#177cff"
                                font.pixelSize: 14; font.bold: true; font.family: "Courier"
                            }
                        }
                        Text {
                            text: fader < 0 ? "REAR" : fader > 0 ? "FRONT" : "CENTRE"
                            color: "#333355"; font.pixelSize: 8; font.letterSpacing: 1
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                // Reset spatial
                Rectangle {
                    width: 100; height: 32; radius: 8
                    color: "#16162a"
                    border.color: "#2a2a44"; border.width: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        anchors.centerIn: parent
                        text: "CENTRE"
                        color: "#444466"
                        font.pixelSize: 10; font.letterSpacing: 2; font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { balance = 0; fader = 0 }
                    }
                }
            }
        }
    }
}
