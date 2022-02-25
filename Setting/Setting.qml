import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls 2.14

import "./SwitchButton"

Rectangle {
    id: mainSetting
    color: "black"

    Rectangle {
        id: title
        width: parent.width
        height: 60 + Config.topMargin
        anchors.left: parent.left
        anchors.top: parent.top
        color: "#2f4f4f"

        MouseArea {
            anchors.fill: parent
        }

        Text {
            id: btnReturn
            width: parent.height - Config.topMargin
            height: width
            y: Config.topMargin

            anchors.left: parent.left
            font.family: "FontAwesome"
            font.pixelSize: width * 0.85
            text: "\uf104"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: btnReturnArea.pressed ? "#6f9f9f" : "white"
            MouseArea {
                id: btnReturnArea
                anchors.fill: parent
                onClicked: {
                    stackView.pop()
                }
            }

            Text {
                anchors.left: parent.right
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: 15
                text: qsTr("Setting")
                color: "white"
            }
        }
    }

    Flickable {
        id: flick
//        anchors.fill: parent
        anchors.left: title.left
        anchors.top: title.bottom
        width: parent.width
        height: parent.height - title.height
        clip: true

        contentHeight: column.implicitHeight

        property real itemWidth: width
        property real itemHeight: 60

        property real sectionHeight: 30

        bottomMargin: Config.bottomMargin

        enum ButtonId {
            WhiteHot = 0,
            BlackHot,
            Iron,
            HCR,
            Rainbow,
            IconGray
        }

        Column {
            id: column
            anchors.fill: parent

            Loader {
                sourceComponent: section
                onLoaded: {
                    item.text = qsTr("Wifi")
                }
            }

            spacing: -1

            function turnToHotspot() {
                PhoneApi.openHotspot()
            }

            Loader {
                sourceComponent: jumpItem
                onLoaded: {
                    item.label = qsTr("Hotspot")
                    item.iconSource = "\uf1e0"
                    item.clicked.connect(parent.turnToHotspot)
                }
            }

            Loader {
                sourceComponent: inputItem
                onLoaded: {
                    item.text = qsTr("SSID")
                    item.value = PhoneApi.hotspotSSID
                }
            }

            Loader {
                sourceComponent: inputItem
                onLoaded: {
                    item.text = qsTr("Password")
                    item.value = PhoneApi.hotspotPassword
                }
            }

            Loader {
                sourceComponent: section
                onLoaded: {
                    item.text = qsTr("Palette")
                }
            }

            function onPaletteClicked(index) {
                TcpCamera.setPalette(index)
            }

            Loader {
                property int currentIndex: TcpCamera.palette
                sourceComponent: selectItem
                onLoaded: {
                    item.label = qsTr("WhiteHot")
                    item.index = 0
                    item.iconSource = "qrc:/sources/whitehot.jpg"
                    item.selectClicked.connect(column.onPaletteClicked)
                }
            }

            Loader {
                property int currentIndex: TcpCamera.palette
                sourceComponent: selectItem
                onLoaded: {
                    item.label = qsTr("BlackHot")
                    item.index = 1
                    item.iconSource = "qrc:/sources/blackhot.jpg"
                    item.selectClicked.connect(column.onPaletteClicked)
                }
            }

            Loader {
                property int currentIndex: TcpCamera.palette
                sourceComponent: selectItem
                onLoaded: {
                    item.label = qsTr("Iron")
                    item.index = 2
                    item.iconSource = "qrc:/sources/iron.jpg"
                    item.selectClicked.connect(column.onPaletteClicked)
                }
            }

            Loader {
                property int currentIndex: TcpCamera.palette
                sourceComponent: selectItem
                onLoaded: {
                    item.label = qsTr("HCR")
                    item.index = 3
                    item.iconSource = "qrc:/sources/HCR.jpg"
                    item.selectClicked.connect(column.onPaletteClicked)
                }
            }

            Loader {
                property int currentIndex: TcpCamera.palette
                sourceComponent: selectItem
                onLoaded: {
                    item.label = qsTr("Rainbow")
                    item.index = 4
                    item.iconSource = "qrc:/sources/rainbow.jpg"
                    item.selectClicked.connect(column.onPaletteClicked)
                }
            }

            Loader {
                property int currentIndex: TcpCamera.palette
                sourceComponent: selectItem
                onLoaded: {
                    item.label = qsTr("IronGray")
                    item.index = 5
                    item.iconSource = "qrc:/sources/irongray.jpg"
                    item.selectClicked.connect(column.onPaletteClicked)
                }
            }

            Loader {
                sourceComponent: section
                onLoaded: {
                    item.text = qsTr("Camera Param")
                }
            }

            Loader {
                id: emissSlider
                sourceComponent: sliderItem
                onLoaded: {
                    item.from = 0.1
                    item.to = 1
                    item.step = 0.01
                    item.value = TcpCamera.emiss
                    item.text = qsTr("Emiss")
                    item.floatLength = 2
                }
            }

            Loader {
                id: reflectedSlider
                sourceComponent: sliderItem
                onLoaded: {
                    item.from = -20
                    item.to = 120
                    item.step = 0.1
                    item.value = TcpCamera.reflected
                    item.text = qsTr("Reflected")
                }
            }

            Loader {
                id: ambientSlider
                sourceComponent: sliderItem
                onLoaded: {
                    item.from = -20
                    item.to = 120
                    item.step = 0.1
                    item.value = TcpCamera.ambient
                    item.text = qsTr("Ambient")
                }
            }

            Loader {
                id: humidnessSlider
                sourceComponent: sliderItem
                onLoaded: {
                    item.from = -20
                    item.to = 120
                    item.step = 0.1
                    item.value = TcpCamera.humidness
                    item.text = qsTr("Humidness")
                }
            }

            Loader {
                id: correctionSlider
                sourceComponent: sliderItem
                onLoaded: {
                    item.from = 0
                    item.to = 3
                    item.step = 0.01
                    item.value = TcpCamera.correction
                    item.text = qsTr("Correction")
                    item.floatLength = 2
                }
            }

            Loader {
                id: distanceSlider
                sourceComponent: sliderItem
                onLoaded: {
                    item.from = 0
                    item.to = 50
                    item.step = 1
                    item.value = TcpCamera.distance
                    item.text = qsTr("Distance")
                }
            }

            function onSaveClicked() {
                messagebox.text = qsTr("Save success")
                console.log(emissSlider.item.value,
                            reflectedSlider.item.value,
                            ambientSlider.item.value,
                            humidnessSlider.item.value,
                            correctionSlider.item.value,
                            distanceSlider.item.value)
                TcpCamera.setCameraParam(emissSlider.item.value,
                                         reflectedSlider.item.value,
                                         ambientSlider.item.value,
                                         humidnessSlider.item.value,
                                         correctionSlider.item.value,
                                         distanceSlider.item.value)
            }

            Loader {
                id: btnSave
                sourceComponent: iconButton
                onLoaded: {
                    item.iconSource = "\uf085"
                    item.label = qsTr("Save")
                    item.clicked.connect(column.onSaveClicked)
                }
            }

            Loader {
                sourceComponent: section
                onLoaded: {
                    item.text = qsTr("Language")
                }
            }

            function onLanguageClicked(index) {
                Config.setLanguage(index)
            }

            Loader {
                property int currentIndex: Config.language
                sourceComponent: selectItem
                onLoaded: {
                    item.label = "简体中文"
                    item.index = 1
                    item.selectClicked.connect(column.onLanguageClicked)
                }
            }

            Loader {
                property int currentIndex: Config.language
                sourceComponent: selectItem
                onLoaded: {
                    item.label = "English"
                    item.index = 0
                    item.selectClicked.connect(column.onLanguageClicked)
                }
            }

            Loader {
                sourceComponent: section
                onLoaded: {
                    item.text = qsTr("Product")
                }
            }

            Loader {
                id: cameraSN
                sourceComponent: infoItem
                onLoaded: {
                    item.text = qsTr("SN")
                    item.value = TcpCamera.cameraSN
                    item.isBottom = true
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            contentItem: Rectangle {
                implicitWidth: 5
                color: "#b0505050"
                radius: implicitWidth / 2
            }
        }

        Component.onCompleted: {
            emissSlider.item.value = TcpCamera.emiss
            reflectedSlider.item.value = TcpCamera.reflected
            ambientSlider.item.value = TcpCamera.ambient
            humidnessSlider.item.value = TcpCamera.humidness
            correctionSlider.item.value = TcpCamera.correction
            distanceSlider.item.value = TcpCamera.distance
            cameraSN.item.value = TcpCamera.cameraSN
        }

        Connections {
            target: TcpCamera
            onCameraSNChanged: {
                cameraSN.item.value = TcpCamera.cameraSN
            }
        }
    }

    Component {
        id: section
        Rectangle {
            property alias text: label.text
            color: "#2f4f4f"
            width: flick.width
            height: flick.sectionHeight

            Text {
                id: label
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: Config.leftMargin > 0 ? Config.leftMargin : 20
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Component {
        id: selectItem
        AbstractButton {
            id: button
            property alias iconSource: icon.source
            property alias label: label.text
            property int index: 0
            property bool isBottom: false
            property real leftRightMargin: 30
            signal selectClicked(var index)

            background: Rectangle {
                color: pressed ? "#606060" : "transparent"
            }

            onClicked: selectClicked(index)

            width: flick.itemWidth
            height: flick.itemHeight

            // 图片图标
            Image {
                id: icon
                asynchronous: true
                width: parent.width > parent.height ? parent.height * 0.75 : parent.width * 0.75
                height: width
                anchors.left: parent.left
                anchors.leftMargin: Config.leftMargin > 0
                                    ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
            }

            // 文本
            Text {
                id: label
                color: "white"
                anchors.left: icon.status !== Image.Null ?
                                  icon.right : parent.left
                anchors.leftMargin: parent.leftRightMargin
                anchors.verticalCenter: icon.verticalCenter
            }

            // 选中图标
            Text {
                id: selectIcon
                anchors.right: parent.right
                anchors.rightMargin: Config.rightMargin > 0
                                    ? Config.rightMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
                font.family: "FontAwesome"
                font.pixelSize: parent.leftRightMargin
                text: "\uf192"
                color: currentIndex === parent.index ?
                           "#ffffff" : "#505050"
            }


            // 底部横条
            Rectangle {
                width: parent.width - label.x - (parent.width - (selectIcon.x + selectIcon.width))
                height: 1
                anchors.left: label.left
                anchors.bottom: parent.bottom
                color: "#606060"
                visible: !parent.isBottom
            }
        }
    }

    Component {
        id: iconButton
        AbstractButton {
            property alias iconSource: icon.text
            property alias label: label.text
            property int index: 0
            property bool isBottom: false
            property real leftRightMargin: 30

            background: Rectangle {
                color: pressed ? "#606060" : "transparent"
            }

            onClicked: console.log("clicked:", index)

            width: flick.itemWidth
            height: flick.itemHeight

            // 文字图标
            Rectangle {
                id: iconBackground
                width: parent.width > parent.height ? parent.height * 0.75 : parent.width * 0.75
                height: width
                color: "#606060"
                radius: width * 0.15
                anchors.left: parent.left
                anchors.leftMargin: Config.leftMargin > 0
                                    ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    id: icon
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: parent.width * 0.55
                    color: "white"
                }
            }

            // 文本
            Text {
                id: label
                color: "white"
                anchors.left: iconBackground.status !== Image.Null ?
                                  iconBackground.right : parent.left
                anchors.leftMargin: parent.leftRightMargin
                anchors.verticalCenter: iconBackground.verticalCenter
            }

            // 底部横条
            Rectangle {
                width: parent.width - iconBackground.width - parent.leftRightMargin * 2
                height: 1
                anchors.left: label.left
                anchors.bottom: parent.bottom
                color: "#606060"
                visible: !parent.isBottom
            }
        }
    }

    Component {
        id: sliderItem
        Rectangle {
            property string text: ""
            property alias value: slider.value
            property alias from: slider.from
            property alias to: slider.to
            property alias step: slider.stepSize
            property int floatLength: 1
            property bool isBottom: false
            property real leftRightMargin: 30

            id: rect
            width: flick.itemWidth
            height: flick.itemHeight
            color: "transparent"

            // 文本
            Text {
                id: label
                text: parent.text + "\n" + slider.value.toFixed(floatLength)
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: Config.leftMargin > 0
                                    ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
            }

            // 滑动条
            Slider {
                id: slider
//                width: parent.width - label.anchors.leftMargin - 20
//                       - anchors.rightMargin - parent.height * 1.5
                width: parent.width
                       - (label.x + parent.width * 0.15 + parent.leftRightMargin)
                       - anchors.rightMargin
                anchors.right: parent.right
                anchors.rightMargin: Config.rightMargin > 0
                                    ? Config.rightMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
            }

            // 底部横条
            Rectangle {
                width: parent.width - label.x - slider.anchors.rightMargin
                height: 1
                anchors.left: label.left
                anchors.bottom: parent.bottom
                color: "#606060"
                visible: !parent.isBottom
            }
        }
    }

    Component {
        id: infoItem
        Item {
            property alias text: text.text
            property alias value: value.text
            property real leftRightMargin: 30
            property bool isBottom: false

            width: flick.itemWidth
            height: flick.itemHeight
            Text {
                id: text
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: Config.leftMargin > 0
                                    ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: value
                color: "white"
                anchors.right: parent.right
                anchors.rightMargin: Config.rightMargin > 0
                                    ? Config.rightMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
            }

            // 底部横条
            Rectangle {
                width: parent.width - text.x - value.anchors.rightMargin
                height: 1
                anchors.left: text.left
                anchors.bottom: parent.bottom
                color: "#606060"
                visible: !parent.isBottom
            }
        }
    }

    Component {
        id: inputItem
        Item {
            property alias text: text.text
            property alias value: value.text
            property real leftRightMargin: 30
            property bool isBottom: false

            width: flick.itemWidth
            height: flick.itemHeight
            Text {
                id: text
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: Config.leftMargin > 0
                                    ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
            }

            TextInput {
                id: value
                width: parent.width * 0.5
                height: parent.height
                anchors.right: parent.right
                anchors.rightMargin: parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
                clip: true
                color: "white"
                verticalAlignment: TextInput.AlignVCenter
                selectByMouse: true

                onActiveFocusChanged: {
                    if( activeFocus ) {
                        cursorColor = "white"
                    }
                    else {
                        cursorColor = "transparent"
                    }
                }

                property color cursorColor: "transparent"
                Behavior on cursorColor {
                    ColorAnimation {
                        duration: 500
                        onRunningChanged: {
                            if( !running && value.activeFocus ) {
                                if( Qt.colorEqual(value.cursorColor, "white") ) {
                                    value.cursorColor = "transparent"
                                }
                                else {
                                    value.cursorColor = "white"
                                }
                            }
                        }
                    }
                }
                cursorDelegate: Rectangle {
                    id: cursor
                    width: 1
                    height: parent.contentHeight - 1
                    color: value.cursorColor
                }
            }

            // 底部横条
            Rectangle {
                width: parent.width - text.anchors.leftMargin - value.anchors.rightMargin
                height: 1
                anchors.left: text.left
                anchors.bottom: parent.bottom
                color: "#606060"
                visible: !parent.isBottom
            }
        }
    }

    Component {
        id: jumpItem
        AbstractButton {
            property alias iconSource: icon.text
            property alias label: label.text
            property real leftRightMargin: 30
            property bool isBottom: false

            width: flick.itemWidth
            height: flick.itemHeight

            background: Rectangle {
                color: pressed ? "#606060" : "transparent"
            }

            // 文字图标
            Rectangle {
                id: iconBackground
                width: parent.width > parent.height ? parent.height * 0.75 : parent.width * 0.75
                height: width
                color: "#606060"
                radius: width * 0.15
                anchors.left: parent.left
                anchors.leftMargin: Config.leftMargin > 0
                                    ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    id: icon
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: parent.width * 0.55
                    color: "white"
                }
            }

            Text {
                id: label
                color: "white"
                anchors.left: iconBackground.right
                anchors.leftMargin: Config.leftMargin > 0
                                    ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
            }

            // icon
            Text {
                id: rightIcon
                anchors.right: parent.right
                anchors.rightMargin: Config.rightMargin > 0
                                    ? Config.rightMargin + parent.leftRightMargin : parent.leftRightMargin
                anchors.verticalCenter: parent.verticalCenter
                font.family: "FontAwesome"
                font.pixelSize: parent.leftRightMargin
                text: "\uf105"
                color: "white"
            }

            // 底部横条
            Rectangle {
                width: parent.width
                       - label.x
                       - rightIcon.anchors.rightMargin
                height: 1
                anchors.left: label.left
                anchors.bottom: parent.bottom
                color: "#606060"
                visible: !parent.isBottom
            }
        }
    }
}
