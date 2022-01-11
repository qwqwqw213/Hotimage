import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.14
//import QtQuick.Controls 1.4

Drawer {
    background: Rectangle{
        color: "#AF000000"
    }

    Flickable {
        id: flick
        anchors.fill: parent
//        contentHeight: itemHeight * 13 + sectionHeight * 3
        contentHeight: column.implicitHeight

        property real itemWidth: width
        property real itemHeight: 60

        property real sectionHeight: 30


        Column {
            id: column
            anchors.fill: parent

            Loader {
                sourceComponent: section
                onLoaded: {
                    item.text = qsTr("Palette")
                }
            }

            Loader {
                sourceComponent: paletteItem
                onLoaded: {
                    item.text = qsTr("WhiteHot")
                    item.paletteIndex = 0
                    item.source = "qrc:/sources/whitehot.jpg"
                }
            }

            Loader {
                sourceComponent: paletteItem
                onLoaded: {
                    item.text = qsTr("BlackHot")
                    item.paletteIndex = 1
                    item.source = "qrc:/sources/blackhot.jpg"
                }
            }

            Loader {
                sourceComponent: paletteItem
                onLoaded: {
                    item.text = qsTr("Iron")
                    item.paletteIndex = 2
                    item.source = "qrc:/sources/iron.jpg"
                }
            }

            Loader {
                sourceComponent: paletteItem
                onLoaded: {
                    item.text = qsTr("HCR")
                    item.paletteIndex = 3
                    item.source = "qrc:/sources/HCR.jpg"
                }
            }

            Loader {
                sourceComponent: paletteItem
                onLoaded: {
                    item.text = qsTr("Rainbow")
                    item.paletteIndex = 4
                    item.source = "qrc:/sources/rainbow.jpg"
                }
            }

            Loader {
                sourceComponent: paletteItem
                onLoaded: {
                    item.text = qsTr("IronGray")
                    item.paletteIndex = 5
                    item.source = "qrc:/sources/irongray.jpg"
                    item.isBottom = true
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

            // 保存按钮
            Rectangle {
                id: btnSave
                width: flick.itemWidth
                height: flick.itemHeight
                color: "transparent"



                MouseArea {
                    anchors.fill: parent
                    enabled: !btnSaveTimer.running

                    function isInvalidParam(value) {
                        if( value < 0.00001 && value > -0.00001 ) {
                            return true
                        }
                        return false
                    }

                    onReleased: {
                        if( emissSlider.item.value < 0.001 && emissSlider.item.value > -0.001 ) {
                            console.log("fail")
                        }

                        if( isInvalidParam(emissSlider.item.value)
                                || isInvalidParam(reflectedSlider.item.value)
                                || isInvalidParam(ambientSlider.item.value)
                                || isInvalidParam(humidnessSlider.item.value) )
                        {
                            messagebox.showMsg(qsTr("Param can't be set zero"))
                        }
                        else
                        {
                            messagebox.showMsg(qsTr("Save success"))
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
                            btnSaveTimer.start()
                        }
                    }
                }
                Timer {
                    id: btnSaveTimer
                    interval: 1000
                }

                Text {
                    id: label
                    color: "white"
                    anchors.centerIn: parent
                    text: qsTr("Save settings")
                }

                Text {
                    font.family: "FontAwesome"
                    font.pixelSize: flick.itemHeight * 0.55
                    text: "\uf085 "
                    anchors.right: label.left
                    anchors.rightMargin: font.pixelSize / 2
                    anchors.verticalCenter: parent.verticalCenter
                    color: "white"
                }
            }

            Loader {
                sourceComponent: section
                onLoaded: {
                    item.text = qsTr("Language")
                }
            }

            Loader {
                sourceComponent: radioItem
                onLoaded: {
                    item.text = "简体中文"
                    item.index = 1
                }
            }

            Loader {
                sourceComponent: radioItem
                onLoaded: {
                    item.isBottom = true
                    item.text = "English"
                    item.index = 0
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
                }
            }
        }
        ScrollIndicator.vertical: ScrollIndicator {

        }

//        onVisibleChanged: {
//            if( visible === true ) {
//                emissSlider.item.value = TcpCamera.emiss
//                reflectedSlider.item.value = TcpCamera.reflected
//                ambientSlider.item.value = TcpCamera.ambient
//                humidnessSlider.item.value = TcpCamera.humidness
//                correctionSlider.item.value = TcpCamera.correction
//                distanceSlider.item.value = TcpCamera.distance
//            }
//        }

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
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Component {
        id: paletteItem
        Rectangle {
            id: item
            property int paletteIndex: 0
            property alias text: label.text
            property alias source: icon.source
            property bool isBottom: false

            width: flick.itemWidth
            height: flick.itemHeight
            color: "transparent"
            Image {
                id: icon
                asynchronous: true
                width: parent.width > parent.height ? parent.height * 0.75 : parent.width * 0.75
                height: width
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: label
                color: "white"
                anchors.left: icon.right
                anchors.leftMargin: icon.width
                anchors.verticalCenter: icon.verticalCenter
            }
            MouseArea {
                anchors.fill: parent
                onReleased: {
                    TcpCamera.setPalette(parent.paletteIndex)
                }
            }

            Rectangle {
                width: parent.width - icon.width * 2 - 20 - 1
                height: 1
                anchors.left: label.left
                anchors.bottom: parent.bottom
                color: "white"
                visible: !parent.isBottom
            }

            // icon
            Text {
                id: selectIcon
                property real iwidth: parent.width > parent.height ?
                                          parent.height * 0.5 : parent.width * 0.5
                anchors.right: parent.right
                anchors.rightMargin: iwidth
                anchors.verticalCenter: parent.verticalCenter
                font.family: "FontAwesome"
                font.pixelSize: iwidth
                text: "\uf192"
//                color: TcpCamera.palette === parent.paletteIndex ? "#ffffff" : "#505050"

                state: "noselection"
                states: [
                    State {
                        name: "selection"
                        when: TcpCamera.palette === item.paletteIndex
                        PropertyChanges {
                            target: selectIcon
                            color: "#ffffff"
                        }
                    },
                    State {
                        name: "noselection"
                        when: TcpCamera.palette !== item.paletteIndex
                        PropertyChanges {
                            target: selectIcon
                            color: "#505050"
                        }
                    }
                ]
                transitions: [
                    Transition {
                        from: "selection"
                        to: "noselection"

                        ColorAnimation {
                            target: selectIcon
                            duration: 200
                        }
                    },
                    Transition {
                        from: "noselection"
                        to: "selection"

                        ColorAnimation {
                            target: selectIcon
                            duration: 200
                        }
                    }
                ]
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

            id: rect
            width: flick.itemWidth
            height: flick.itemHeight
            color: "transparent"

            Slider {
                id: slider
                width: parent.width - parent.height * 1.5 - 40
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: label
                text: parent.text + "\n" + slider.value.toFixed(floatLength)
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Component {
        id: radioItem
        Rectangle {
            id: item
            property int index: 0
            property alias text: label.text
            property bool isBottom: false

            width: flick.itemWidth
            height: flick.itemHeight
            color: "transparent"
            Text {
                id: label
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }
            MouseArea {
                anchors.fill: parent
                onReleased: {
                    Config.setLanguage(parent.index)
                }
            }
            Rectangle {
                width: parent.width - 40
                height: 1
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.bottom: parent.bottom
                color: "white"
                visible: !parent.isBottom
            }

            // icon
            Text {
                id: selectIcon
                property real iwidth: parent.width > parent.height ?
                                          parent.height * 0.5 : parent.width * 0.5
                anchors.right: parent.right
                anchors.rightMargin: iwidth
                anchors.verticalCenter: parent.verticalCenter
                font.family: "FontAwesome"
                font.pixelSize: iwidth
                text: "\uf192"
//                color: TcpCamera.palette === parent.paletteIndex ? "#ffffff" : "#505050"

                state: "noselection"
                states: [
                    State {
                        name: "selection"
                        when: Config.language === item.index
                        PropertyChanges {
                            target: selectIcon
                            color: "#ffffff"
                        }
                    },
                    State {
                        name: "noselection"
                        when: Config.language !== item.index
                        PropertyChanges {
                            target: selectIcon
                            color: "#505050"
                        }
                    }
                ]
                transitions: [
                    Transition {
                        from: "selection"
                        to: "noselection"

                        ColorAnimation {
                            target: selectIcon
                            duration: 200
                        }
                    },
                    Transition {
                        from: "noselection"
                        to: "selection"

                        ColorAnimation {
                            target: selectIcon
                            duration: 200
                        }
                    }
                ]
            }
        }
    }

    Component {
        id: infoItem
        Rectangle {
            property alias text: text.text
            property alias value: value.text
            width: flick.itemWidth
            height: flick.itemHeight
            color: "transparent"
            Text {
                id: text
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: value
                color: "white"
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
