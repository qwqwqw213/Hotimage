import QtQuick 2.4

Item {
    id: scaleBar
    clip: true

    signal zoomScaleChanged()

    property real percent
    property real value: 1.0
    property real textRotation: 0

    Rectangle {
        id: zoomScaleBackground
        width: parent.width
        height: parent.height
        color: "#a0000000"

        y: height
        opacity: 0.0

        Behavior on y {
            YAnimator {
                duration: 200
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }

        Rectangle {
            id: zoomScale
            width: parent.width
            height: parent.height

            property real maxX: width / 2
            property real minX: 0 - width / 2

            x: maxX
            color: "transparent"

            Text {
                text: "3.0"
                color: "yellow"
                font.pixelSize: parent.height * 0.2
                y: 8 + parent.height * 0.4
                x: parent.width - contentWidth / 2
                opacity: zoomScale.x < zoomScale.minX + (contentWidth * 2) ?
                             (1 - ((zoomScale.minX + (contentWidth * 2)) - zoomScale.x) / (contentWidth * 2)) : 1
                scale: opacity
                rotation: scaleBar.textRotation
            }

            Text {
                text: "2.0"
                color: "yellow"
                font.pixelSize: parent.height * 0.2
                y: 8 + parent.height * 0.4
                x: parent.width / 2 - contentWidth / 2
                opacity: Math.abs(zoomScale.x) / (contentWidth * 2) <= 1 ?
                             Math.abs(zoomScale.x) / (contentWidth * 2) : 1
                scale: opacity
                rotation: scaleBar.textRotation
            }

            Text {
                text: "1.0"
                color: "yellow"
                font.pixelSize: parent.height * 0.2
                y: 8 + parent.height * 0.4
                x: 0 - contentWidth / 2
                opacity: zoomScale.x > zoomScale.maxX - (contentWidth * 2) ?
                             (1 - (zoomScale.x - (zoomScale.maxX - (contentWidth * 2))) / (contentWidth * 2)) : 1
                scale: opacity
                rotation: scaleBar.textRotation
            }

            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.lineWidth = 1

                    for(var i = 0; i <= 20; i ++)
                    {
                        ctx.beginPath()
                        if( (i % 10) === 0 ) {
                            ctx.strokeStyle = "#ffffff"
                        }
                        else {
                            ctx.strokeStyle = "#a0a0a0"
                        }

                        var x = width / 20 * (20 - i)
                        ctx.moveTo(x, 8)
                        ctx.lineTo(x, height * 0.4)
                        ctx.stroke()
                    }
                }
            }
        }
    }

    Rectangle {
        id: textBackground
        color: "#50000000"
        anchors.centerIn: parent
        anchors.verticalCenterOffset: (parent.height * 0.6 - width) / 2

        width: text.font.pixelSize * 2
        height: width
        radius: width / 2
        opacity: zoomScaleBackground.opacity < 0.5 ? 1 : 0

        Behavior on opacity {
            OpacityAnimator {
                duration: 100
            }
        }
    }

    Text {
        id: text
        color: "white"
        anchors.centerIn: textBackground
        font.pixelSize: parent.height * 0.2 * (0.7 + 0.3 * zoomScaleBackground.opacity)
        text: scaleBar.value.toFixed(1)
        rotation: scaleBar.textRotation
        opacity: zoomScaleBackground.opacity > 0.6 ?
                     zoomScaleBackground.opacity : (0.6 + zoomScaleBackground.opacity)
    }

    Timer {
        id: timer
        interval: 1500
        onTriggered: {
            zoomScaleBackground.opacity = 0.0
            zoomScaleBackground.y = zoomScaleBackground.height
        }
    }

    MouseArea {
        anchors.fill: parent

        property real currentX
        property real pressedX
        onPressed: {
            pressedX = mouseX
            currentX = zoomScale.x

            zoomScaleBackground.opacity = 1
            zoomScaleBackground.y = 0
            timer.start()
        }

        onMouseXChanged: {
            timer.restart()
            var x = mouseX - pressedX + currentX
            if( x < zoomScale.minX ) {
                x = zoomScale.minX
            }
            if( x > zoomScale.maxX ) {
                x = zoomScale.maxX
            }

            zoomScale.x = x
            parent.percent = (x - zoomScale.minX) / (zoomScale.maxX - zoomScale.minX)
            parent.value = 3.0 - 2.0 * parent.percent

            parent.zoomScaleChanged()
        }

        onReleased: {

        }
    }
}
