import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FluentUI 1.0

Rectangle {
    id: root
    color: FluTheme.dark ? "#09111f" : "#f3f6fb"
    property bool wideLayout: width >= 1380
    property bool mediumLayout: width >= 980

    function toneColor(name) {
        if (name === "success") return "#0f7b6c"
        if (name === "warning") return "#b7791f"
        if (name === "danger") return "#c42b1c"
        return "#4f8cff"
    }

    function textColor(level) {
        if (level === "hero") return FluTheme.dark ? "#f8fbff" : "#0f172a"
        if (level === "primary") return FluTheme.dark ? "#e7efff" : "#162033"
        if (level === "secondary") return FluTheme.dark ? "#a6b3c8" : "#56657c"
        return FluTheme.dark ? "#7d8ca6" : "#8090a6"
    }

    function emphasisColor(level, accent) {
        if (level === "primary") return FluTheme.dark ? "#f8fbff" : "#0f172a"
        if (level === "accent") return accent || "#4f8cff"
        return textColor("secondary")
    }

    function historyForKind(kind) {
        if (kind === "cpu") return backend.cpuHistory
        if (kind === "gpu") return backend.gpuHistory
        if (kind === "memory") return backend.memoryHistory
        if (kind === "network") return backend.networkHistory
        return []
    }

    function componentLabel(kind) {
        if (kind === "cpu") return "CPU"
        if (kind === "gpu") return "GPU"
        if (kind === "memory") return "Memory"
        if (kind === "network") return "Network"
        return "Module"
    }

    gradient: Gradient {
        GradientStop { position: 0.0; color: FluTheme.dark ? "#08111d" : "#f9fbff" }
        GradientStop { position: 0.45; color: FluTheme.dark ? "#0b1727" : "#edf3fb" }
        GradientStop { position: 1.0; color: FluTheme.dark ? "#0d1423" : "#f3f6fb" }
    }

    component Sparkline: Item {
        id: sparkline
        property var points: []
        property color strokeColor: "#4f8cff"
        property color fillColor: Qt.rgba(0.31, 0.55, 1.0, 0.14)
        property real maxValue: 100

        implicitHeight: 96
        clip: true

        Canvas {
            id: canvas
            anchors.fill: parent
            antialiasing: true
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()

                var values = sparkline.points || []
                var w = width
                var h = height
                if (w <= 0 || h <= 0)
                    return

                ctx.strokeStyle = Qt.rgba(1, 1, 1, FluTheme.dark ? 0.08 : 0.35)
                ctx.lineWidth = 1
                for (var i = 1; i <= 3; ++i) {
                    var gy = (h / 4) * i
                    ctx.beginPath()
                    ctx.moveTo(0, gy)
                    ctx.lineTo(w, gy)
                    ctx.stroke()
                }

                if (values.length === 0)
                    return

                function pointX(index) {
                    return values.length === 1 ? w : (w / (values.length - 1)) * index
                }

                function pointY(value) {
                    var bounded = Math.max(0, Math.min(sparkline.maxValue, Number(value)))
                    return h - ((bounded / sparkline.maxValue) * (h - 8)) - 4
                }

                ctx.beginPath()
                ctx.moveTo(pointX(0), h)
                for (var a = 0; a < values.length; ++a)
                    ctx.lineTo(pointX(a), pointY(values[a]))
                ctx.lineTo(pointX(values.length - 1), h)
                ctx.closePath()
                ctx.fillStyle = sparkline.fillColor
                ctx.fill()

                ctx.beginPath()
                for (var b = 0; b < values.length; ++b) {
                    var x = pointX(b)
                    var y = pointY(values[b])
                    if (b === 0)
                        ctx.moveTo(x, y)
                    else
                        ctx.lineTo(x, y)
                }
                ctx.strokeStyle = sparkline.strokeColor
                ctx.lineWidth = 2.5
                ctx.stroke()

                var lastX = pointX(values.length - 1)
                var lastY = pointY(values[values.length - 1])
                ctx.beginPath()
                ctx.arc(lastX, lastY, 4, 0, 2 * Math.PI)
                ctx.fillStyle = sparkline.strokeColor
                ctx.fill()
            }

            Connections {
                target: sparkline
                function onPointsChanged() { canvas.requestPaint() }
                function onStrokeColorChanged() { canvas.requestPaint() }
                function onFillColorChanged() { canvas.requestPaint() }
            }

            Component.onCompleted: requestPaint()
        }
    }

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: Math.max(root.width - 12, 360)
            spacing: 22

            Item { Layout.fillWidth: true; Layout.preferredHeight: 24 }

            FluFrame {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                Layout.preferredHeight: mediumLayout ? 208 : 268

                Rectangle {
                    anchors.fill: parent
                    radius: 4
                    color: "transparent"
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: FluTheme.dark ? "#102f59" : "#dce9ff" }
                        GradientStop { position: 0.5; color: FluTheme.dark ? "#123b4e" : "#d9f4f0" }
                        GradientStop { position: 1.0; color: FluTheme.dark ? "#2b2254" : "#eef0ff" }
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 28
                    spacing: 18

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 24

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            FluText { text: "HxMonitor"; font.pixelSize: 14; font.weight: Font.DemiBold; color: root.textColor("primary") }
                            FluText { text: "Telemetry cockpit"; font.pixelSize: mediumLayout ? 38 : 30; font.weight: Font.Black; color: root.textColor("hero") }

                            FluText {
                                Layout.fillWidth: true
                                text: "Group hardware data by system role, bring critical numbers forward, and keep the short-term trend visible."
                                wrapMode: Text.WordWrap
                                color: root.textColor("secondary")
                            }
                        }

                        ColumnLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignTop
                            FluFilledButton { visible: backend.debugMode; Layout.preferredWidth: 168; text: "Open log viewer"; onClicked: backend.openLogViewer() }
                            FluButton { Layout.preferredWidth: 168; text: "Refresh media"; onClicked: backend.refreshMediaList() }
                            FluTextButton { Layout.preferredWidth: 168; text: FluTheme.dark ? "Use light mode" : "Use dark mode"; onClicked: FluTheme.darkMode = FluTheme.dark ? FluThemeType.Light : FluThemeType.Dark }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: mediumLayout ? 2 : 1
                        columnSpacing: 12
                        rowSpacing: 12

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 54
                            radius: 18
                            color: FluTheme.dark ? "#152033" : Qt.rgba(255, 255, 255, 0.75)
                            border.color: FluTheme.dark ? "#2c405f" : "#dbe7f5"

                            Column {
                                anchors.centerIn: parent
                                spacing: 4
                                FluText { anchors.horizontalCenter: parent.horizontalCenter; text: "IPC"; color: root.textColor("tertiary"); font.pixelSize: 12; font.weight: Font.DemiBold }
                                FluText { anchors.horizontalCenter: parent.horizontalCenter; text: backend.nmStatusText; font.pixelSize: 14; font.weight: Font.Bold; color: root.textColor("hero") }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 54
                            radius: 18
                            color: FluTheme.dark ? "#152033" : Qt.rgba(255, 255, 255, 0.75)
                            border.color: FluTheme.dark ? "#2c405f" : "#dbe7f5"

                            Column {
                                anchors.centerIn: parent
                                spacing: 4
                                FluText { anchors.horizontalCenter: parent.horizontalCenter; text: "Serial"; color: root.textColor("tertiary"); font.pixelSize: 12; font.weight: Font.DemiBold }
                                FluText { anchors.horizontalCenter: parent.horizontalCenter; text: backend.serialConnected ? "Connected" : "Standby"; font.pixelSize: 14; font.weight: Font.Bold; color: root.textColor("hero") }
                            }
                        }
                    }
                }
            }

            FluFrame {
                id: performanceFrame
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                Layout.preferredHeight: performanceContent.implicitHeight + 48
                clip: true

                ColumnLayout {
                    id: performanceContent
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 18

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        FluText { text: backend.performanceOverview.eyebrow || "Performance cockpit"; font.pixelSize: 13; font.weight: Font.DemiBold; color: "#4f8cff" }
                        FluText { text: backend.performanceOverview.title || "System performance"; font.pixelSize: 30; font.weight: Font.Black; color: root.textColor("hero") }
                        FluText { text: backend.performanceOverview.description || ""; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: root.textColor("secondary") }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: wideLayout ? 4 : 2
                            columnSpacing: 12
                            rowSpacing: 12

                            Repeater {
                                model: backend.performanceOverview.summaryMetrics || []
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 82
                                    radius: 20
                                    color: FluTheme.dark ? "#111b2d" : "#f8fbff"
                                    border.color: FluTheme.dark ? "#24324a" : "#dae4f1"
                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 5
                                        FluText { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.label; font.pixelSize: 12; font.weight: Font.DemiBold; color: root.textColor("tertiary") }
                                        FluText { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.value; font.pixelSize: 22; font.weight: Font.Black; color: root.emphasisColor(modelData.emphasis, "#4f8cff") }
                                    }
                                }
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: wideLayout ? 2 : 1
                        columnSpacing: 18
                        rowSpacing: 18

                        Repeater {
                            model: backend.componentCards
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 316
                                radius: 24
                                color: FluTheme.dark ? "#0f1828" : "#ffffff"
                                border.color: FluTheme.dark ? "#25324b" : "#dde5f0"
                                clip: true

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 18
                                    spacing: 14

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 10

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 14

                                            Rectangle {
                                                Layout.preferredWidth: 12
                                                Layout.preferredHeight: 60
                                                radius: 6
                                                color: modelData.accent
                                                Layout.alignment: Qt.AlignTop
                                            }
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                spacing: 4
                                                FluText { text: componentLabel(modelData.kind); font.pixelSize: 12; font.weight: Font.DemiBold; color: modelData.accent }
                                                FluText { text: modelData.title; font.pixelSize: 24; font.weight: Font.Black; color: root.textColor("hero"); elide: Text.ElideRight }
                                                FluText { text: modelData.subtitle; color: root.textColor("secondary"); elide: Text.ElideRight }
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 18
                                            ColumnLayout {
                                                spacing: 4
                                                FluText { text: "Priority"; font.pixelSize: 11; font.weight: Font.DemiBold; color: root.textColor("tertiary") }
                                                FluText { text: modelData.primaryValue || "--"; font.pixelSize: 28; font.weight: Font.Black; color: root.textColor("hero") }
                                            }
                                            ColumnLayout {
                                                spacing: 4
                                                visible: (modelData.secondaryValue || "") !== ""
                                                FluText { text: "Secondary"; font.pixelSize: 11; font.weight: Font.DemiBold; color: root.textColor("tertiary") }
                                                FluText { text: modelData.secondaryValue; font.pixelSize: 18; font.weight: Font.DemiBold; color: root.textColor("primary") }
                                            }
                                            ColumnLayout {
                                                spacing: 4
                                                visible: (modelData.statusText || "") !== ""
                                                FluText { text: "Status"; font.pixelSize: 11; font.weight: Font.DemiBold; color: root.textColor("tertiary") }
                                                FluText { text: modelData.statusText; font.pixelSize: 18; font.weight: Font.DemiBold; color: modelData.accent }
                                            }
                                        }
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 132
                                        Layout.minimumHeight: 132
                                        radius: 18
                                        color: FluTheme.dark ? "#111a29" : "#f8fbff"
                                        border.color: FluTheme.dark ? "#22324a" : "#e1e8f1"
                                        clip: true

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 14
                                            spacing: 10

                                            FluText { text: "Recent trend"; font.pixelSize: 11; font.weight: Font.DemiBold; color: root.textColor("tertiary") }
                                            Sparkline {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                Layout.minimumHeight: 84
                                                points: historyForKind(modelData.kind)
                                                strokeColor: modelData.accent
                                                fillColor: FluTheme.dark ? "#1b2f55" : "#d9e8ff"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            GridLayout {
                id: operationsGrid
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                columns: width >= 1440 ? 3 : mediumLayout ? 2 : 1
                columnSpacing: 18
                rowSpacing: 18

                FluFrame {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 280
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 14
                        RowLayout {
                            Layout.fillWidth: true
                            FluText { text: "Device bridge"; font.pixelSize: 24; font.weight: Font.Black; color: root.textColor("hero") }
                            Item { Layout.fillWidth: true }
                            Rectangle {
                                radius: 11
                                color: backend.serialConnected ? (FluTheme.dark ? "#123c35" : "#dff6ed") : (FluTheme.dark ? "#3d2d14" : "#fff4d8")
                                border.color: backend.serialConnected ? "#0f7b6c" : "#c58b00"
                                implicitWidth: 96
                                implicitHeight: 34
                                FluText { anchors.centerIn: parent; text: backend.serialConnected ? "Online" : "Standby"; color: backend.serialConnected ? "#0f7b6c" : "#9a6700"; font.weight: Font.Bold }
                            }
                        }
                        FluText { text: "Send the grouped telemetry payload to the selected serial target."; wrapMode: Text.WordWrap; color: root.textColor("secondary") }
                        ComboBox { Layout.fillWidth: true; model: backend.serialPorts; currentIndex: Math.max(0, backend.serialPorts.indexOf(backend.selectedPort)); onActivated: backend.setSelectedPort(currentText) }
                        FluText { text: backend.selectedPort === "" ? "No serial port selected" : backend.selectedPort; color: root.textColor("tertiary"); elide: Text.ElideRight }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluButton { Layout.fillWidth: true; text: "Refresh ports"; onClicked: backend.refreshPorts() }
                            FluFilledButton { Layout.fillWidth: true; text: backend.serialConnected ? "Disconnect" : "Connect"; onClicked: backend.toggleSerialConnection() }
                        }
                    }
                }

                FluFrame {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 280
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 12
                        RowLayout {
                            Layout.fillWidth: true
                            FluText { text: "Native messaging"; font.pixelSize: 24; font.weight: Font.Black; color: root.textColor("hero") }
                            Item { Layout.fillWidth: true }
                            Rectangle { Layout.preferredWidth: 12; Layout.preferredHeight: 12; radius: 6; color: root.toneColor(backend.nmStatusTone) }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 72
                            radius: 18
                            color: FluTheme.dark ? "#101927" : "#f8fbff"
                            border.color: root.toneColor(backend.nmStatusTone)
                            Column {
                                anchors.centerIn: parent
                                spacing: 6
                                FluText { anchors.horizontalCenter: parent.horizontalCenter; text: backend.nmStatusText; font.pixelSize: 15; font.weight: Font.Bold; color: root.textColor("primary") }
                                FluText { anchors.horizontalCenter: parent.horizontalCenter; text: "Sent " + backend.msgSentCount + " / Recv " + backend.msgRecvCount; color: root.textColor("secondary") }
                            }
                        }
                        FluText { text: "Use this area to confirm whether the browser proxy is attached and exchanging commands."; wrapMode: Text.WordWrap; color: root.textColor("secondary") }
                        Item { Layout.fillHeight: true }
                        FluButton { Layout.fillWidth: true; text: "Refresh media inventory"; onClicked: backend.refreshMediaList() }
                    }
                }

                FluFrame {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 280
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 12
                        RowLayout {
                            Layout.fillWidth: true
                            FluText { text: "Media quick control"; font.pixelSize: 24; font.weight: Font.Black; color: root.textColor("hero") }
                            Item { Layout.fillWidth: true }
                            FluFilledButton { text: "Refresh"; onClicked: backend.refreshMediaList() }
                        }
                        FluText { text: backend.mediaItems.length === 0 ? "No active media tabs detected." : "The current active tab can be controlled directly from here."; wrapMode: Text.WordWrap; color: root.textColor("secondary") }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 88
                            radius: 18
                            color: FluTheme.dark ? "#101927" : "#f8fbff"
                            border.color: FluTheme.dark ? "#26334a" : "#dde5f0"
                            Column {
                                anchors.centerIn: parent
                                spacing: 5
                                width: parent.width - 24
                                FluText {
                                    width: parent.width
                                    horizontalAlignment: Text.AlignHCenter
                                    text: backend.mediaItems.length > 0 ? backend.mediaItems[0].title : "Waiting for media"
                                    font.pixelSize: 16
                                    font.weight: Font.Bold
                                    color: root.textColor("primary")
                                    maximumLineCount: 2
                                    wrapMode: Text.WordWrap
                                    elide: Text.ElideRight
                                }
                                FluText { anchors.horizontalCenter: parent.horizontalCenter; text: backend.mediaItems.length > 0 ? backend.mediaItems[0].timeline : "No timeline"; color: root.textColor("secondary") }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            FluButton { Layout.fillWidth: true; text: "Prev"; onClicked: backend.playPrevious() }
                            FluFilledButton { Layout.fillWidth: true; text: "Play"; onClicked: backend.playPause() }
                            FluButton { Layout.fillWidth: true; text: "Next"; onClicked: backend.playNext() }
                            FluButton { Layout.fillWidth: true; text: "PiP"; onClicked: backend.togglePip() }
                        }
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 28
                Layout.rightMargin: 28
                columns: backend.debugMode && root.width >= 1360 ? 2 : 1
                columnSpacing: 18
                rowSpacing: 18

                FluFrame {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 430
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 14
                        FluText { text: "Media sessions"; font.pixelSize: 24; font.weight: Font.Black; color: root.textColor("hero") }
                        FluText { text: "Each media source stays in a clean row with the title first and the transport controls second."; wrapMode: Text.WordWrap; color: root.textColor("secondary") }
                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            spacing: 10
                            model: backend.mediaItems
                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 96
                                radius: 18
                                color: modelData.active ? (FluTheme.dark ? "#142238" : "#ecf4ff") : (FluTheme.dark ? "#111927" : "#ffffff")
                                border.color: modelData.active ? "#4f8cff" : (FluTheme.dark ? "#25324b" : "#dde5f0")
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 14
                                    Rectangle {
                                        Layout.preferredWidth: 54
                                        Layout.preferredHeight: 54
                                        radius: 16
                                        color: modelData.playing ? "#dff6ed" : "#fff4d8"
                                        border.color: modelData.playing ? "#0f7b6c" : "#c58b00"
                                        FluText { anchors.centerIn: parent; text: modelData.playing ? "On" : "Off"; font.pixelSize: 16; font.weight: Font.Black; color: modelData.playing ? "#0f7b6c" : "#9a6700" }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        FluText { text: modelData.title; font.pixelSize: 18; font.weight: Font.Black; color: root.textColor("hero"); elide: Text.ElideRight }
                                        FluText { text: modelData.artist; font.pixelSize: 13; color: root.textColor("secondary"); elide: Text.ElideRight }
                                        FluText { text: modelData.stateLabel + "  |  " + modelData.timeline; font.pixelSize: 12; font.weight: Font.DemiBold; color: modelData.active ? "#4f8cff" : root.textColor("tertiary") }
                                    }
                                    Item { Layout.fillWidth: true }
                                    FluButton {
                                        text: "Toggle"
                                        Layout.preferredWidth: 92
                                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                                        onClicked: backend.playPause(modelData.tabId)
                                    }
                                }
                                MouseArea { anchors.fill: parent; acceptedButtons: Qt.LeftButton; onDoubleClicked: backend.activateMedia(modelData.tabId) }
                            }
                        }
                    }
                }

                FluFrame {
                    visible: backend.debugMode
                    Layout.fillWidth: true
                    Layout.preferredHeight: 430
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 14
                        FluText { text: "IPC log stream"; font.pixelSize: 24; font.weight: Font.Black; color: root.textColor("hero") }
                        FluText { text: "Recent log lines stay isolated in a console-style panel so protocol problems are easier to scan."; wrapMode: Text.WordWrap; color: root.textColor("secondary") }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 18
                            color: FluTheme.dark ? "#09111c" : "#f8fbff"
                            border.color: FluTheme.dark ? "#25324b" : "#dde5f0"
                            ListView {
                                anchors.fill: parent
                                anchors.margins: 12
                                clip: true
                                spacing: 8
                                model: backend.nmLogs
                                delegate: Rectangle {
                                    width: ListView.view.width
                                    height: logText.paintedHeight + 18
                                    radius: 12
                                    color: FluTheme.dark ? "#101927" : "#ffffff"
                                    border.color: FluTheme.dark ? "#25324b" : "#e1e8f1"
                                    Text {
                                        id: logText
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        text: modelData
                                        wrapMode: Text.WrapAnywhere
                                        color: FluTheme.dark ? "#d5e2f3" : "#243247"
                                        font.family: "Consolas"
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true; Layout.preferredHeight: 28 }
        }
    }
}
