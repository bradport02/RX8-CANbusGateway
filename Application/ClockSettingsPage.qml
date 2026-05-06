import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool use24h:    systemClock ? systemClock.use24h : true
    property int  dispHour:   0
    property int  dispMinute: 0
    property int  dispSecond: 0
    property int  dispYear:   2024
    property int  dispMonth:  1
    property int  dispDay:    1
    property bool dispPm:     false
    property string statusMessage: ""

    // Days in current month
    function daysInMonth(y, m) {
        return new Date(y, m, 0).getDate()
    }

    function pullCurrentTime() {
        var d = new Date()
        dispHour   = d.getHours()
        dispMinute = d.getMinutes()
        dispSecond = d.getSeconds()
        dispYear   = d.getFullYear()
        dispMonth  = d.getMonth() + 1
        dispDay    = d.getDate()
        dispPm     = dispHour >= 12
    }

    function pad(n)       { return n < 10 ? "0" + n : "" + n }
    function displayHour() {
        if (use24h) return pad(dispHour)
        var h = dispHour % 12; return pad(h === 0 ? 12 : h)
    }

    function incHour()   { dispHour   = (dispHour   + 1) % 24; dispPm = dispHour >= 12 }
    function decHour()   { dispHour   = (dispHour   + 23) % 24; dispPm = dispHour >= 12 }
    function incMinute() { dispMinute = (dispMinute + 1) % 60 }
    function decMinute() { dispMinute = (dispMinute + 59) % 60 }
    function incSecond() { dispSecond = (dispSecond + 1) % 60 }
    function decSecond() { dispSecond = (dispSecond + 59) % 60 }
    function incDay()    {
        var max = daysInMonth(dispYear, dispMonth)
        dispDay = dispDay >= max ? 1 : dispDay + 1
    }
    function decDay()    {
        var max = daysInMonth(dispYear, dispMonth)
        dispDay = dispDay <= 1 ? max : dispDay - 1
    }
    function incMonth()  {
        dispMonth = dispMonth >= 12 ? 1 : dispMonth + 1
        var max = daysInMonth(dispYear, dispMonth)
        if (dispDay > max) dispDay = max
    }
    function decMonth()  {
        dispMonth = dispMonth <= 1 ? 12 : dispMonth - 1
        var max = daysInMonth(dispYear, dispMonth)
        if (dispDay > max) dispDay = max
    }
    function incYear()   { dispYear++ }
    function decYear()   { if (dispYear > 2000) dispYear-- }
    function togglePm()  {
        dispPm = !dispPm
        dispHour = dispPm ? (dispHour % 12) + 12 : dispHour % 12
    }

    function applyDateTime() {
        // Send to STM32 RTC — format: 0 = 24h, 1 = 12h
        uartController.sendTime(dispHour, dispMinute, dispSecond,
                                dispDay, dispMonth, dispYear,
                                use24h ? 0 : 1)

        // Also update the Pi's system clock if you still want local sync
        if (systemClock)
            systemClock.setDateTime(dispYear, dispMonth, dispDay,
                                    dispHour, dispMinute, dispSecond)

        statusMessage = "Set to " + dispYear + "-" + pad(dispMonth) + "-" + pad(dispDay)
                      + "  " + pad(dispHour) + ":" + pad(dispMinute) + ":" + pad(dispSecond)
        statusTimer.restart()
    }

    Component.onCompleted: {
        // Connect incoming RTC time from STM32
        uartController.rtcTimeReceived.connect(function(hour, min, sec,
                                                         day, month, year, format) {
            dispHour   = hour
            dispMinute = min
            dispSecond = sec
            dispDay    = day
            dispMonth  = month
            dispYear   = year
            // format: 0=24h, 1=12h — only honour if you want the STM32 to drive this
            // if (systemClock) systemClock.setUse24h(format === 0)

            // Sync Pi system clock to the RTC value
            if (systemClock)
                systemClock.setDateTime(year, month, day, hour, min, sec)

            statusMessage = "Synced from RTC"
            statusTimer.restart()
        })

        // Request current time from STM32 RTC on startup
        uartController.send(0x1D,0); //Send a request time packet
    }

    // ── Background ────────────────────────────────────────────────────────────
    Rectangle { anchors.fill: parent; color: "#111118" }

    // ── 12h / 24h toggle ─────────────────────────────────────────────────────
    Row {
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        Repeater {
            model: [{ label: "24h", val: true }, { label: "12h", val: false }]
            Rectangle {
                width: 90; height: 34; radius: 6
                color: (use24h === modelData.val) ? "#177cff" : "#1e1e2e"
                border.color: (use24h === modelData.val) ? "#177cff" : "#333355"
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: (use24h === modelData.val) ? "white" : "#666688"
                    font.pixelSize: 14; font.bold: (use24h === modelData.val)
                }
                MouseArea { anchors.fill: parent; onClicked: { if (systemClock) systemClock.setUse24h(modelData.val) } }
            }
        }
    }

    // ── Digit column component ────────────────────────────────────────────────
    component DigitColumn: Column {
        id: col
        property string value: "00"
        property string label: ""
        property int    digitWidth: 72
        signal increment()
        signal decrement()
        spacing: 6

        Rectangle {
            width: col.digitWidth; height: 40; radius: 8
            color: upMouse.pressed ? "#177cff" : "#1e1e2e"
            border.color: "#2a2a4a"; border.width: 1
            anchors.horizontalCenter: parent.horizontalCenter
            Text { anchors.centerIn: parent; text: "▲"; font.pixelSize: 20
                   color: upMouse.pressed ? "white" : "#7ab3ff" }
            MouseArea {
                id: upMouse; anchors.fill: parent
                onClicked: col.increment()
                onPressAndHold: upRepeat.start()
                onReleased: upRepeat.stop()
                Timer { id: upRepeat; interval: 110; repeat: true; onTriggered: col.increment() }
            }
        }

        Rectangle {
            width: col.digitWidth + 8; height: 72; radius: 10
            color: "#0d0d1a"; border.color: "#2a3a6a"; border.width: 2
            anchors.horizontalCenter: parent.horizontalCenter
            Text { anchors.centerIn: parent; text: col.value; color: "white"
                   font.pixelSize: 36; font.bold: true; font.family: "Courier" }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: col.label; color: "#444466"; font.pixelSize: 10
            font.capitalization: Font.AllUppercase; font.letterSpacing: 2
        }

        Rectangle {
            width: col.digitWidth; height: 40; radius: 8
            color: downMouse.pressed ? "#177cff" : "#1e1e2e"
            border.color: "#2a2a4a"; border.width: 1
            anchors.horizontalCenter: parent.horizontalCenter
            Text { anchors.centerIn: parent; text: "▼"; font.pixelSize: 20
                   color: downMouse.pressed ? "white" : "#7ab3ff" }
            MouseArea {
                id: downMouse; anchors.fill: parent
                onClicked: col.decrement()
                onPressAndHold: downRepeat.start()
                onReleased: downRepeat.stop()
                Timer { id: downRepeat; interval: 110; repeat: true; onTriggered: col.decrement() }
            }
        }
    }

    // ── Separator ─────────────────────────────────────────────────────────────
    Text {
        id: sectionSep
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.18
        text: "TIME"
        color: "#333355"
        font.pixelSize: 10
        font.letterSpacing: 4
        font.capitalization: Font.AllUppercase
    }

    // ── Time row ──────────────────────────────────────────────────────────────
    Row {
        id: timeRow
        anchors.top: sectionSep.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 4

        DigitColumn { value: displayHour();  label: "hour"; onIncrement: incHour();   onDecrement: decHour()   }
        Text { text: ":"; color: "#7ab3ff"; font.pixelSize: 44; font.bold: true; anchors.verticalCenter: parent.verticalCenter; anchors.verticalCenterOffset: 6 }
        DigitColumn { value: pad(dispMinute); label: "min";  onIncrement: incMinute(); onDecrement: decMinute() }
        Text { text: ":"; color: "#7ab3ff"; font.pixelSize: 44; font.bold: true; anchors.verticalCenter: parent.verticalCenter; anchors.verticalCenterOffset: 6 }
        DigitColumn { value: pad(dispSecond); label: "sec";  onIncrement: incSecond(); onDecrement: decSecond() }

        // AM/PM
        Column {
            visible: !use24h
            spacing: 6
            anchors.verticalCenter: parent.verticalCenter
            Repeater {
                model: [{ t: "AM", pm: false }, { t: "PM", pm: true }]
                Rectangle {
                    width: 56; height: 38; radius: 8
                    color: (dispPm === modelData.pm) ? "#177cff" : "#1e1e2e"
                    border.color: "#2a2a4a"; border.width: 1
                    Text { anchors.centerIn: parent; text: modelData.t
                           color: (dispPm === modelData.pm) ? "white" : "#666688"
                           font.pixelSize: 13; font.bold: (dispPm === modelData.pm) }
                    MouseArea { anchors.fill: parent; onClicked: if (dispPm !== modelData.pm) togglePm() }
                }
            }
        }
    }

    // ── Date section label ────────────────────────────────────────────────────
    Text {
        id: dateSep
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: timeRow.bottom
        anchors.topMargin: 16
        text: "DATE"
        color: "#333355"
        font.pixelSize: 10
        font.letterSpacing: 4
        font.capitalization: Font.AllUppercase
    }

    // ── Date row ──────────────────────────────────────────────────────────────
    Row {
        id: dateRow
        anchors.top: dateSep.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8

        DigitColumn { value: pad(dispDay);   label: "day";   digitWidth: 64; onIncrement: incDay();   onDecrement: decDay()   }
        DigitColumn { value: pad(dispMonth); label: "month"; digitWidth: 64; onIncrement: incMonth(); onDecrement: decMonth() }
        DigitColumn { value: String(dispYear); label: "year"; digitWidth: 88; onIncrement: incYear(); onDecrement: decYear()  }
    }

    // ── Action buttons ────────────────────────────────────────────────────────
    Row {
        anchors.bottom: statusLabel.top
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 16

        Rectangle {
            width: 130; height: 42; radius: 8
            color: "#1e1e2e"; border.color: "#333355"; border.width: 1
            Text { anchors.centerIn: parent; text: "Reset"; color: "#aaaacc"; font.pixelSize: 14 }
            MouseArea { anchors.fill: parent; onClicked: pullCurrentTime() }
        }

        Rectangle {
            width: 130; height: 42; radius: 8
            color: "#0a2a5a"; border.color: "#177cff"; border.width: 1
            Text { anchors.centerIn: parent; text: "Apply"; color: "#7ab3ff"; font.pixelSize: 14; font.bold: true }
            MouseArea { anchors.fill: parent; onClicked: applyDateTime() }
        }
    }

    // ── Status ────────────────────────────────────────────────────────────────
    Text {
        id: statusLabel
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 14
        anchors.horizontalCenter: parent.horizontalCenter
        text: statusMessage
        color: "#2aff7a"; font.pixelSize: 12
        opacity: statusMessage !== "" ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 400 } }
        Timer { id: statusTimer; interval: 3000; onTriggered: statusMessage = "" }
    }
}
