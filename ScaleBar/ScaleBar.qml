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

        x: width
        opacity: 0.0

        Behavior on x {
            XAnimator {
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

            property real minY: 0 - height / 2
            property real maxY: height / 2

            y: 0 - height / 2
//            color: "#ff0000"
            color: "transparent"

            Text {
                text: "3.0"
                color: "yellow"
                font.pixelSize: parent.width * 0.2
                anchors.rightMargin:  (parent.width * 0.6 - contentWidth) / 2
                anchors.right: parent.right
                y: 0 - contentHeight / 2
                opacity: zoomScale.y > zoomScale.maxY - (contentHeight * 2) ?
                             (1 - (zoomScale.y - (zoomScale.maxY - (contentHeight * 2))) / (contentHeight * 2)) : 1
                scale: opacity
                rotation: scaleBar.textRotation
            }

            Text {
                text: "2.0"
                color: "yellow"
                font.pixelSize: parent.width * 0.2
                anchors.rightMargin:  (parent.width * 0.6 - contentWidth) / 2
                anchors.right: parent.right
                y: parent.height / 2 - contentHeight / 2
                opacity: Math.abs(zoomScale.y) / (contentHeight * 2) <= 1 ?
                             Math.abs(zoomScale.y) / (contentHeight * 2) : 1
                scale: opacity
                rotation: scaleBar.textRotation
            }

            Text {
                text: "1.0"
                color: "yellow"
                font.pixelSize: parent.width * 0.2
                anchors.rightMargin:  (parent.width * 0.6 - contentWidth) / 2
                anchors.right: parent.right
                y: parent.height - contentHeight / 2
                opacity: zoomScale.y < zoomScale.minY + (contentHeight * 2) ?
                             (1 - ((zoomScale.minY + (contentHeight * 2)) - zoomScale.y) / (contentHeight * 2)) : 1
                scale: opacity
                rotation: scaleBar.textRotation
            }

            Canvas {
                anchors.fill: parent
//                smooth: true
//                antialiasing: true
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

                        var y = height / 20 * (20 - i)
                        ctx.moveTo(8, y)
                        ctx.lineTo(width * 0.4, y)
                        ctx.stroke()
                    }
                }
            }
        }
    }

    Rectangle {
        id: textBackground
        color: "#50000000"
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin:  (parent.width * 0.6 - width) / 2
        anchors.right: parent.right

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
        font.pixelSize: parent.width * 0.2 * (0.7 + 0.3 * zoomScaleBackground.opacity)
        text: scaleBar.value.toFixed(1)
        rotation: scaleBar.textRotation
        opacity: zoomScaleBackground.opacity > 0.6 ?
                     zoomScaleBackground.opacity : (0.6 + zoomScaleBackground.opacity)
    }

    Timer {
        id: timer
        interval: 2000
        onTriggered: {
            zoomScaleBackground.opacity = 0.0
            zoomScaleBackground.x = zoomScaleBackground.width
        }
    }

    MouseArea {
        anchors.fill: parent

        property real currentY
        property real pressedY
        onPressed: {
            pressedY = mouseY
            currentY = zoomScale.y

            zoomScaleBackground.opacity = 1
            zoomScaleBackground.x = 0
            timer.start()
        }

        onMouseYChanged: {
            timer.restart()
            var y = mouseY - pressedY + currentY
            if( y < zoomScale.minY ) {
                y = zoomScale.minY
            }
            if( y > zoomScale.maxY ) {
                y = zoomScale.maxY
            }

            zoomScale.y = y
            parent.percent = (y - zoomScale.minY) / (zoomScale.maxY - zoomScale.minY)
//                console.log(parent.percent)
            parent.value = 1.0 + 2.0 * parent.percent

            parent.zoomScaleChanged()
        }

        onReleased: {

        }
    }
}
