import QtQuick
import QtQuick.Controls

Rectangle {
    id: ambientLightingPage
    color: "#1a1a1a"

    // ── Exposed properties (bind these to your lighting controller) ──────────
    property int   selectedR:   255
    property int   selectedG:   100
    property int   selectedB:   0
    property int   brightness:  80
    property bool  lightsOn:    true


    // ── Internal helpers ─────────────────────────────────────────────────────
    readonly property color previewColour: {
        if (!lightsOn) return "#1a1a1a"
        var f = brightness / 100.0
        return Qt.rgba(selectedR/255 * f, selectedG/255 * f, selectedB/255 * f, 1.0)
    }

    function hexFromRGB(r, g, b) {
        return "#" + r.toString(16).padStart(2,"0").toUpperCase()
                   + g.toString(16).padStart(2,"0").toUpperCase()
                   + b.toString(16).padStart(2,"0").toUpperCase()
    }

    // Notify C++ controller whenever colour or brightness changes
    onSelectedRChanged:   ambientController.setColour(selectedR, selectedG, selectedB)
    onSelectedGChanged:   ambientController.setColour(selectedR, selectedG, selectedB)
    onSelectedBChanged:   ambientController.setColour(selectedR, selectedG, selectedB)
    onBrightnessChanged:  ambientController.setBrightness(brightness)
    onLightsOnChanged:    ambientController.setPower(lightsOn)

    // ── Colour presets data ──────────────────────────────────────────────────
    property var presets: [
        { name: "Ice",    r: 100, g: 200, b: 255 },
        { name: "Fire",   r: 255, g: 60,  b: 0   },
        { name: "Lime",   r: 80,  g: 255, b: 80  },
        { name: "Violet", r: 160, g: 60,  b: 255 },
        { name: "Sunset", r: 255, g: 140, b: 0   },
        { name: "Rose",   r: 255, g: 60,  b: 120 },
        { name: "Arctic", r: 0,   g: 220, b: 255 },
        { name: "White",  r: 255, g: 255, b: 255 }
    ]

    // ════════════════════════════════════════════════════════════════════════
    // HEADER
    // ════════════════════════════════════════════════════════════════════════
    Rectangle {
        id: header
        width: parent.width
        height: 56
        color: "#222222"
        z: 10

        Text {
            text: "Ambient Lighting"
            color: "white"
            font.pixelSize: 17
            font.bold: true
            anchors.centerIn: parent
        }
    }

    // ════════════════════════════════════════════════════════════════════════
    // CONTENT ROW  (wheel panel  |  divider  |  controls panel)
    // ════════════════════════════════════════════════════════════════════════
    Row {
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        // ── LEFT: Colour Wheel + RGB readout ─────────────────────────────────
        Item {
            id: leftPanel
            width: parent.width * 0.52
            height: parent.height

            // ── Colour Wheel (gradient layers, reliable in Qt QML) ───────────
            Item {
                id: wheelArea
                width: 220
                height: 220
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 20

                // ── Layer 1: Conical hue gradient (full colour ring) ──────────
                // Qt's ConicalGradient goes clockwise from the 'angle' position.
                // We stack 6 rectangle slices to approximate a smooth hue wheel,
                // then mask to a circle with an OpacityMask / layer trick.
                // Simpler and guaranteed to work: use Canvas arc segments for hue.
                Canvas {
                    id: wheelCanvas
                    anchors.fill: parent
                    antialiasing: true

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)

                        var cx   = width  / 2
                        var cy   = height / 2
                        var r    = Math.min(cx, cy) - 1
                        var step = 1   // degrees per segment — fine enough, fast enough

                        // ── Draw hue ring as thin arc segments ────────────────
                        for (var angle = 0; angle < 360; angle += step) {
                            var startRad = (angle - 0.5) * Math.PI / 180
                            var endRad   = (angle + step + 0.5) * Math.PI / 180

                            // Radial gradient: white centre → pure hue at edge
                            var grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, r)
                            grad.addColorStop(0,   "white")
                            grad.addColorStop(1,   "hsl(" + angle + ", 100%, 50%)")

                            ctx.beginPath()
                            ctx.moveTo(cx, cy)
                            ctx.arc(cx, cy, r, startRad, endRad)
                            ctx.closePath()
                            ctx.fillStyle = grad
                            ctx.fill()
                        }
                    }

                    // ── Colour math: hue/sat from polar coords ────────────────
                    function hsvToRgb(h, s, v) {
                        var r, g, b
                        var i = Math.floor(h / 60) % 6
                        var f = h / 60 - Math.floor(h / 60)
                        var p = v * (1 - s)
                        var q = v * (1 - f * s)
                        var t = v * (1 - (1 - f) * s)
                        switch (i) {
                            case 0: r=v; g=t; b=p; break
                            case 1: r=q; g=v; b=p; break
                            case 2: r=p; g=v; b=t; break
                            case 3: r=p; g=q; b=v; break
                            case 4: r=t; g=p; b=v; break
                            case 5: r=v; g=p; b=q; break
                        }
                        return { r: Math.round(r*255), g: Math.round(g*255), b: Math.round(b*255) }
                    }

                    function pickAt(px, py) {
                        var cx  = width  / 2
                        var cy  = height / 2
                        var r   = Math.min(cx, cy) - 1
                        var dx  = px - cx
                        var dy  = py - cy
                        var dist = Math.sqrt(dx*dx + dy*dy)
                        if (dist > r) return

                        // Hue: atan2 with 0° at right (east), going counter-clockwise
                        // hsl() in canvas also uses 0°=red going counter-clockwise, so match exactly
                        var hue = Math.atan2(dy, dx) * 180 / Math.PI
                        if (hue < 0) hue += 360

                        var sat = dist / r
                        // White blend: colour = hue at sat=1, white at sat=0, v always 1
                        var rgb = hsvToRgb(hue, sat, 1.0)

                        // Blend toward white based on (1 - sat)
                        selectedR = Math.round(rgb.r + (255 - rgb.r) * (1 - sat))
                        selectedG = Math.round(rgb.g + (255 - rgb.g) * (1 - sat))
                        selectedB = Math.round(rgb.b + (255 - rgb.b) * (1 - sat))

                        cursor.x = px - cursor.width  / 2
                        cursor.y = py - cursor.height / 2
                    }

                    MouseArea {
                        anchors.fill: parent
                        onPressed:         wheelCanvas.pickAt(mouseX, mouseY)
                        onPositionChanged: if (pressed) wheelCanvas.pickAt(mouseX, mouseY)
                    }
                }

                // Cursor indicator
                Rectangle {
                    id: cursor
                    width: 20
                    height: 20
                    radius: 10
                    color: Qt.rgba(selectedR/255, selectedG/255, selectedB/255, 1)
                    border.color: "white"
                    border.width: 3
                    x: wheelArea.width/2  - width/2
                    y: wheelArea.height/2 - height/2
                    z: 10
                }

                // Outer ring border
                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: "transparent"
                    border.color: "#444444"
                    border.width: 2
                    z: 5
                }
            }

            // ── RGB Value Chips ──────────────────────────────────────────────
            Row {
                id: rgbRow
                anchors.top: wheelArea.bottom
                anchors.topMargin: 18
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 10

                Repeater {
                    model: [
                        { label: "R", colour: "#ff6b6b", value: selectedR },
                        { label: "G", colour: "#6bff8a", value: selectedG },
                        { label: "B", colour: "#6baeff", value: selectedB }
                    ]
                    delegate: Rectangle {
                        width: 68
                        height: 58
                        color: "#222222"
                        border.color: "#333333"
                        border.width: 1
                        radius: 10

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: modelData.label
                                color: modelData.colour
                                font.pixelSize: 10
                                font.bold: true
                                font.letterSpacing: 2
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: modelData.value
                                color: "white"
                                font.pixelSize: 22
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }
            }

            // ── HEX Label ────────────────────────────────────────────────────
            Rectangle {
                id: hexChip
                anchors.top: rgbRow.bottom
                anchors.topMargin: 10
                anchors.horizontalCenter: parent.horizontalCenter
                width: 180
                height: 34
                color: "#222222"
                border.color: "#333333"
                border.width: 1
                radius: 10

                Text {
                    anchors.centerIn: parent
                    text: "HEX   " + hexFromRGB(selectedR, selectedG, selectedB)
                    color: "white"
                    font.pixelSize: 13
                    font.bold: true
                    font.letterSpacing: 1
                }
            }
        }

        // ── DIVIDER ───────────────────────────────────────────────────────────
        Rectangle {
            width: 1
            height: parent.height
            color: "#2a2a2a"
        }

        // ── RIGHT: Controls ───────────────────────────────────────────────────
        Item {
            id: rightPanel
            width: parent.width - leftPanel.width - 1
            height: parent.height

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 16

                // Preview strip
                Column {
                    width: parent.width
                    spacing: 6

                    Text {
                        text: "PREVIEW"
                        color: "#555d72"
                        font.pixelSize: 9
                        font.bold: true
                        font.letterSpacing: 2.5
                    }

                    Rectangle {
                        width: parent.width
                        height: 36
                        radius: 10
                        color: previewColour
                        border.color: lightsOn
                            ? Qt.rgba(selectedR/255, selectedG/255, selectedB/255, 0.4)
                            : "#2a2a2a"
                        border.width: 1

                        Text {
                            visible: !lightsOn
                            anchors.centerIn: parent
                            text: "LIGHTS OFF"
                            color: "#444444"
                            font.pixelSize: 9
                            font.bold: true
                            font.letterSpacing: 2
                        }
                    }
                }

                // Power toggle — full width, sits between preview and brightness
                Column {
                    width: parent.width
                    spacing: 6

                    Text {
                        text: "POWER"
                        color: "#555d72"
                        font.pixelSize: 9
                        font.bold: true
                        font.letterSpacing: 2.5
                    }

                    Rectangle {
                        width: parent.width
                        height: 36
                        radius: 10
                        color: lightsOn ? "#1a3a6a" : "#222222"
                        border.color: lightsOn ? "#4a6fc7" : "#333333"
                        border.width: 1

                        Row {
                            anchors.centerIn: parent
                            spacing: 8

                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: lightsOn ? "#7ab3ff" : "#444444"
                            }

                            Text {
                                text: lightsOn ? "ON" : "OFF"
                                color: lightsOn ? "#7ab3ff" : "#666666"
                                font.pixelSize: 13
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: lightsOn = !lightsOn
                        }
                    }
                }

                // Brightness slider
                Column {
                    width: parent.width
                    spacing: 8

                    // Label row
                    Row {
                        width: parent.width

                        Text {
                            text: "BRIGHTNESS"
                            color: "#555d72"
                            font.pixelSize: 9
                            font.bold: true
                            font.letterSpacing: 2.5
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Item { width: parent.width - brightnessLabel.width - 90; height: 1 }

                        Text {
                            id: brightnessLabel
                            text: brightness + "%"
                            color: "white"
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    // Custom slider built from a MouseArea so there are no
                    // Qt Controls Slider binding-loop issues
                    Item {
                        id: brightnessTrackArea
                        width: parent.width
                        height: 28     // tall enough for easy touch/click

                        // Track background
                        Rectangle {
                            id: brightnessTrack
                            width: parent.width
                            height: 6
                            radius: 3
                            anchors.verticalCenter: parent.verticalCenter

                            // Dark-to-colour gradient, updates with selected colour
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#222222" }
                                GradientStop {
                                    position: 1.0
                                    color: Qt.rgba(selectedR / 255, selectedG / 255, selectedB / 255, 1)
                                }
                            }
                        }

                        // Filled portion (left of handle)
                        Rectangle {
                            width: brightnessHandle.x + brightnessHandle.width / 2
                            height: 6
                            radius: 3
                            anchors.verticalCenter: parent.verticalCenter
                            color: "transparent"   // gradient on full track is enough
                        }

                        // Handle
                        Rectangle {
                            id: brightnessHandle
                            width: 22
                            height: 22
                            radius: 11
                            color: "white"
                            border.color: "#aaaaaa"
                            border.width: 2
                            anchors.verticalCenter: parent.verticalCenter
                            // Position driven purely by brightness value — no Slider binding loop
                            x: (brightness / 100) * (brightnessTrackArea.width - width)

                            // Drop shadow ring using a slightly larger semi-transparent circle
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width + 6
                                height: parent.height + 6
                                radius: width / 2
                                color: "transparent"
                                border.color: Qt.rgba(selectedR/255, selectedG/255, selectedB/255, 0.35)
                                border.width: 3
                                z: -1
                            }
                        }

                        // MouseArea handles drag and click
                        MouseArea {
                            anchors.fill: parent
                            preventStealing: true

                            function updateFromX(mx) {
                                var clamped = Math.max(0, Math.min(mx, brightnessTrackArea.width))
                                brightness  = Math.round((clamped / brightnessTrackArea.width) * 100)
                            }

                            onPressed:         updateFromX(mouseX)
                            onPositionChanged: updateFromX(mouseX)
                        }
                    }
                }

                // Colour Presets
                Column {
                    width: parent.width
                    spacing: 8

                    Text {
                        text: "PRESETS"
                        color: "#555d72"
                        font.pixelSize: 9
                        font.bold: true
                        font.letterSpacing: 2.5
                    }

                    Grid {
                        columns: 4
                        spacing: 8
                        width: parent.width

                        Repeater {
                            model: presets
                            delegate: Rectangle {
                                width:  (parent.width - 24) / 4
                                height: width
                                radius: 10
                                color:  Qt.rgba(modelData.r/255, modelData.g/255, modelData.b/255, 1)
                                border.color: (selectedR === modelData.r &&
                                               selectedG === modelData.g &&
                                               selectedB === modelData.b) ? "white" : "transparent"
                                border.width: 2

                                ToolTip.visible: swatchMouse.containsMouse
                                ToolTip.text:    modelData.name
                                ToolTip.delay:   500

                                MouseArea {
                                    id: swatchMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        selectedR = modelData.r
                                        selectedG = modelData.g
                                        selectedB = modelData.b
                                        // Reset wheel cursor to centre
                                        cursor.x = wheelArea.width/2  - cursor.width/2
                                        cursor.y = wheelArea.height/2 - cursor.height/2
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
