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
        contentHeight: itemHeight * 13 + sectionHeight * 2

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
                    item.from = 0
                    item.to = 1
                    item.step = 0.01
                    item.value = tcpCamera.emiss
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
                    item.value = tcpCamera.reflected
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
                    item.value = tcpCamera.ambient
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
                    item.value = tcpCamera.humidness
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
                    item.value = tcpCamera.correction
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
                    item.value = tcpCamera.distance
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
                    onReleased: {
                        console.log(emissSlider.item.value,
                                    reflectedSlider.item.value,
                                    ambientSlider.item.value,
                                    humidnessSlider.item.value,
                                    correctionSlider.item.value,
                                    distanceSlider.item.value)
                        tcpCamera.setCameraParam(emissSlider.item.value,
                                                 reflectedSlider.item.value,
                                                 ambientSlider.item.value,
                                                 humidnessSlider.item.value,
                                                 correctionSlider.item.value,
                                                 distanceSlider.item.value)
                        btnSaveTimer.start()
                    }
                }
                Timer {
                    id: btnSaveTimer
                    interval: 1000
                }

                Label {
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
        }
        ScrollIndicator.vertical: ScrollIndicator {

        }

        onVisibleChanged: {
            if( visible === true ) {
                emissSlider.item.value = tcpCamera.emiss
                reflectedSlider.item.value = tcpCamera.reflected
                ambientSlider.item.value = tcpCamera.ambient
                humidnessSlider.item.value = tcpCamera.humidness
                correctionSlider.item.value = tcpCamera.correction
                distanceSlider.item.value = tcpCamera.distance
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

            Label {
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
            Label {
                id: label
                color: "white"
                anchors.left: icon.right
                anchors.leftMargin: icon.width
                anchors.verticalCenter: icon.verticalCenter
            }
            MouseArea {
                anchors.fill: parent
                onReleased: {
                    tcpCamera.setPalette(parent.paletteIndex)
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
//                color: tcpCamera.palette === parent.paletteIndex ? "#ffffff" : "#505050"

                state: "noselection"
                states: [
                    State {
                        name: "selection"
                        when: tcpCamera.palette === item.paletteIndex
                        PropertyChanges {
                            target: selectIcon
                            color: "#ffffff"
                        }
                    },
                    State {
                        name: "noselection"
                        when: tcpCamera.palette !== item.paletteIndex
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

            Label {
                id: label
                text: parent.text + "\n" + slider.value.toFixed(floatLength)
                color: "white"
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
