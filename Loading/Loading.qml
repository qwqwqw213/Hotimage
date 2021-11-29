import QtQuick 2.0

import QtQuick.Shapes 1.14
import QtQuick 2.14

Item {
    id: loading
    property alias text: label.text
    Rectangle {
        anchors.fill: parent
        color: "#f0505050"

        Rectangle {
            id: rect
            width: parent.width > parent.height ? parent.height * 0.5 : parent.width * 0.5
            height: width
            anchors.centerIn: parent
            color: "transparent"
            Shape {
                id: shape
                property real penWidth: 10
                property real angle: 0
                property real arc: 0

                anchors.centerIn: parent
                width: parent.width - 10
                height: parent.height - 10
                layer.enabled: true
                layer.samples: 4
                smooth: true
                antialiasing: true
                ShapePath {
                    id: shapePath
                    fillColor: "transparent"
                    strokeColor: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
                    strokeWidth: shape.penWidth
                    capStyle: ShapePath.FlatCap
                    PathAngleArc {
                        radiusX: shape.width / 2 - shape.penWidth / 2 // 圆弧横向半径
                        radiusY: shape.width / 2 - shape.penWidth / 2 // 圆弧纵向半径
                        centerX: shape.width / 2
                        centerY: shape.width / 2
                        startAngle: shape.angle
                        sweepAngle: shape.arc
                        onStartAngleChanged: {
                            if( sweepAngle < 1 ) {
                                shapePath.strokeColor = Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
                            }
                        }
                    }
                }
            }

            SequentialAnimation
            {
                running: loading.visible
                loops: Animation.Infinite
                NumberAnimation {
                    target: shape
                    property: "arc"
                    duration: 500
                    from: 0
                    to: 180
                    easing.type: Easing.Linear
                }

                NumberAnimation {
                    target: shape
                    property: "angle"
                    duration: 500
                    from: 0
                    to: 180
                    easing.type: Easing.OutCurve
                }

                ParallelAnimation {
                    NumberAnimation {
                        target: shape
                        property: "arc"
                        duration: 800
                        from: 180
                        to: 0
                        easing.type: Easing.OutCurve
                    }

                    NumberAnimation {
                        target: shape
                        property: "angle"
                        duration: 800
                        from: 180
                        to: 360
                        easing.type: Easing.OutCurve
                    }
                }
            }
        }


        Text {
            id: label
            anchors.horizontalCenter: rect.horizontalCenter
            anchors.top: rect.bottom
            anchors.topMargin: 20
            font.pixelSize: 20
            color: "white"
        }
    }

    MouseArea {
        anchors.fill: parent
    }
}
