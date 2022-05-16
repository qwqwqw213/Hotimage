import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.14

import "../BackTitle"

Rectangle {
    color: "black"

    property real leftRightMargin: 30

    BackTitle {
        id: title
        width: parent.width
        height: 60 + Config.topMargin
        titleText: qsTr("Setting")
        buttonText: qsTr("Save")
        onButtonClicked: {
            Config.saveSetting()
            stackView.pop()
        }
    }

    Flickable {
        anchors.left: title.left
        anchors.top: title.bottom
        width: parent.width
        height: parent.height - title.height
        clip: true
        contentHeight: column.implicitHeight
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
            spacing: -1

            property real itemHeight: 60

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Hotspot")
            }

            InputDelegate {
                id: ssid
                height: parent.itemHeight
                label: qsTr("SSID")
                value: TcpCamera.hotspotSSID
            }

            InputDelegate {
                id: password
                height: parent.itemHeight
                label: qsTr("Password")
                value: TcpCamera.hotspotPassword
            }

            SwcDelegate {
                height: parent.itemHeight
                label: qsTr("Hotspot enable")
                onOffStatus: TcpCamera.hotspotEnable
                onClicked: {
                    if( TcpCamera.isConnected ) {
                        if( !TcpCamera.setHotspotParam(ssid.value, password.value) ) {
                            ssid.value = ""
                            password.value = ""
                        }
                    }
                    else {
                        messagebox.text = qsTr("Device not connect")
                        ssid.value = ""
                        password.value = ""
                    }
                }
            }

            Text {
                width: parent.width - leftRightMargin * 2
                height: contentHeight + leftRightMargin * 2
                anchors.left: parent.left
                anchors.leftMargin: leftRightMargin
                topPadding: leftRightMargin
                color: "white"
                wrapMode: Text.Wrap
                lineHeight: 1.5
                text: qsTr("If you want camera device connect to phone hotspot.")
                      + qsTr("\n1. connect camera device wi-fi")
                      + qsTr("\n2. fill in hotspot ssid and password, turn on \"Hotspot enable\"")
                      + qsTr("\n3. open phone system setting, turn on hotspot switch")
                      + qsTr("\nif hotspot info changed, need reconnect device and reset ssid and password")
            }

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Wireless")
            }

            InputDelegate {
                height: parent.itemHeight
                label: qsTr("Device IP")
                checkLabel: qsTr("conn")
                value: TcpCamera.deviceIp
                onCheckClicked: {
                    TcpCamera.manualConnect(value)
                }
            }

            InfoDelegate {
                height: parent.itemHeight
                label: qsTr("Local IP")
                value: TcpCamera.localIp
            }

            Text {
                width: parent.width - leftRightMargin * 2
                height: contentHeight + leftRightMargin * 2
                anchors.left: parent.left
                anchors.leftMargin: leftRightMargin
                topPadding: leftRightMargin
                color: "white"
                wrapMode: Text.Wrap
                lineHeight: 1.5
                text: qsTr("Manual connect camera device, camera device default ip 192.168.1.1")
            }

//            JumpDelegate {
//                height: parent.itemHeight
//                label: qsTr("Hotspot")
//                iconSource: "\uf064"
//                onClicked: {
//                    if( Config.isMobile ) {
//                        PhoneApi.openHotspot()
//                    }
//                }
//            }

//            JumpDelegate {
//                height: parent.itemHeight
//                label: qsTr("Wireless Setting")
//                iconSource: "\uf013"
//                onClicked: {
//                    stackView.push("qrc:/Setting/WirelessPage.qml")
//                }
//            }

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Imaging quality")
            }

            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("FPS first")
                selection: TcpCamera.frameMode === 0 ? true : false
                onClicked: TcpCamera.frameMode = 0
            }

            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("Image first")
                selection: TcpCamera.frameMode === 1 ? true : false
                onClicked: TcpCamera.frameMode = 1
            }

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Palette")
            }


            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("WhiteHot")
                iconSource: "qrc:/resource/whitehot.jpg"
                selection: TcpCamera.palette === 0 ? true : false
                onClicked: TcpCamera.setPalette(0)
            }


            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("BlackHot")
                iconSource: "qrc:/resource/blackhot.jpg"
                selection: TcpCamera.palette === 1 ? true : false
                onClicked: TcpCamera.setPalette(1)
            }

            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("Iron")
                iconSource: "qrc:/resource/iron.jpg"
                selection: TcpCamera.palette === 2 ? true : false
                onClicked: TcpCamera.setPalette(2)
            }

            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("HCR")
                iconSource: "qrc:/resource/HCR.jpg"
                selection: TcpCamera.palette === 3 ? true : false
                onClicked: TcpCamera.setPalette(3)
            }

            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("Rainbow")
                iconSource: "qrc:/resource/rainbow.jpg"
                selection: TcpCamera.palette === 4 ? true : false
                onClicked: TcpCamera.setPalette(4)
            }

            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("IronGray")
                iconSource: "qrc:/resource/irongray.jpg"
                selection: TcpCamera.palette === 5 ? true : false
                onClicked: TcpCamera.setPalette(5)
            }

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Camera Param")
                visible: Config.canReadTemperature
            }

            Loader {
                id: emissSlider
                property real pValue: TcpCamera.emiss
                property real pTo: 1
                property real pFrom: 0.1
                property real pStep: 0.1
                active: Config.canReadTemperature
                sourceComponent: sliderItem
                onLoaded: {
                    item.text = qsTr("Emiss")
                    item.floatLength = 2
                }
            }

            Loader {
                id: reflectedSlider
                property real pValue: TcpCamera.reflected
                property real pTo: 120
                property real pFrom: -20
                property real pStep: 0.1
                active: Config.canReadTemperature
                sourceComponent: sliderItem
                onLoaded: {
                    item.text = qsTr("Reflected")
                }
            }

            Loader {
                id: ambientSlider
                property real pValue: TcpCamera.ambient
                property real pTo: 120
                property real pFrom: -20
                property real pStep: 0.1
                active: Config.canReadTemperature
                sourceComponent: sliderItem
                onLoaded: {
                    item.text = qsTr("Ambient")
                }
            }

            Loader {
                id: humidnessSlider
                property real pValue: TcpCamera.humidness
                property real pTo: 120
                property real pFrom: -20
                property real pStep: 0.1
                active: Config.canReadTemperature
                sourceComponent: sliderItem
                onLoaded: {
                    item.text = qsTr("Humidness")
                }
            }

            Loader {
                id: correctionSlider
                property real pValue: TcpCamera.correction
                property real pTo: 3
                property real pFrom: 0
                property real pStep: 0.1
                active: Config.canReadTemperature
                sourceComponent: sliderItem
                onLoaded: {
                    item.text = qsTr("Correction")
                    item.floatLength = 2
                }
            }

            Loader {
                id: distanceSlider
                property real pValue: TcpCamera.distance
                property real pTo: 50
                property real pFrom: 0
                property real pStep: 1
                active: Config.canReadTemperature
                sourceComponent: sliderItem
                onLoaded: {
                    item.text = qsTr("Distance")
                }
            }

            function onSaveClicked() {
                messagebox.text = qsTr("Save success")
                TcpCamera.setCameraParam(emissSlider.item.value,
                                         reflectedSlider.item.value,
                                         ambientSlider.item.value,
                                         humidnessSlider.item.value,
                                         correctionSlider.item.value,
                                         distanceSlider.item.value)
            }

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Image flip")
            }

            Repeater {
                model: TcpCamera.rotationCount
                SelectDelegate {
                    height: parent.itemHeight
                    label: TcpCamera.rotation(index)
                    selection: TcpCamera.rotationIndex === index
                    onClicked: TcpCamera.rotationIndex = index
                }
            }

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Language")
            }

            SelectDelegate {
                height: parent.itemHeight
                label: qsTr("简体中文")
                selection: Config.language === 1 ? true : false
                onClicked: Config.setLanguage(1)
            }

            SelectDelegate {
                height: parent.itemHeight
                label: "English"
                selection: Config.language === 0 ? true : false
                onClicked: Config.setLanguage(0)
            }

            SectionDelegate {
                height: parent.itemHeight / 2
                label: qsTr("Product")
            }

            InfoDelegate {
                height: parent.itemHeight
                label: qsTr("SN")
                value: TcpCamera.cameraSN
            }
        }

        ScrollBar.vertical: ScrollBar {
            contentItem: Rectangle {
                implicitWidth: 5
                color: "#b0505050"
                radius: implicitWidth / 2
            }
        }
    }

    Component {
        id: sliderItem
        Rectangle {
            property string text: ""
//            property alias value: slider.value
            property real value: pValue
            property real to: pTo
            property real from: pFrom
            property real step: pStep
//            property alias from: slider.from
//            property alias to: slider.to
//            property alias step: slider.stepSize
            property int floatLength: 1
            property real leftRightMargin: 30

            id: rect
            width: column.width
            height: column.itemHeight
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
                value: parent.value
                from: parent.from
                to: parent.to
                stepSize: parent.step
            }

            // 底部横条
            Rectangle {
                width: parent.width - label.x - slider.anchors.rightMargin
                height: 1
                anchors.left: label.left
                anchors.bottom: parent.bottom
                color: "#606060"
            }
        }
    }
}
