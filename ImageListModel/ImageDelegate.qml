import QtQuick 2.14

MouseArea {
    id: imageDelegate

    signal pinchStarted
    signal clicked

    property url imageSource
    property real enterLeaveRatio: flick.width / width
    property bool landscape: width > height ? true : false
    property real w2hRatio: width / height
    property real h2wRatio: height / width
    property bool moving: false
    property int delegateIndex: -1
    property real dragRatio: 1 - Math.abs(flick.contentY / flick.height)

    Flickable {
        id: flick
        clip: true

        contentWidth: width
        contentHeight: height

        PinchArea {
            width: flick.contentWidth
            height: flick.contentHeight

            property real pressedWidth
            property real pressedHeight
            property real minScaleWidth: imageDelegate.width * 0.85
            property real minScaleHeight: imageDelegate.height * 0.85

            onPinchStarted: {
                resizeAnimation.running = false
                pressedWidth = flick.contentWidth
                pressedHeight = flick.contentHeight
                imageDelegate.pinchStarted()
            }

            onPinchUpdated: (pinch) => {
                // adjust content pos due to drag
               flick.contentX += pinch.previousCenter.x - pinch.center.x
               flick.contentY += pinch.previousCenter.y - pinch.center.y

               // resize content
               flick.resizeContent((pressedWidth * pinch.scale) < minScaleWidth ?
                                       minScaleWidth : (pressedWidth * pinch.scale),
                                   (pressedHeight * pinch.scale) < minScaleHeight ?
                                       minScaleHeight : (pressedHeight * pinch.scale), pinch.center)
            }

            onPinchFinished: {
                if( flick.contentWidth < flick.width
                        || flick.contentHeight < flick.height ) {
                    resizeAnimation.resize(flick.x, flick.y, flick.width, flick.height)
                }
            }

            Image {
                id: image
                anchors.centerIn: parent

                property bool binding: false
                function bind() {
                    if( !binding ) {
                        binding = true
                        image.width = Qt.binding(function() {
                            return image.sourceSize.width * (flick.contentWidth / flick.width) * ratio
                        })
                        image.height = Qt.binding(function() {
                            return image.sourceSize.height * (flick.contentWidth / flick.width) * ratio
                        })
                    }
                }

                source: VideoPlayer.playIndex === photoScan.currentIndex ?
                            VideoPlayer.frameUrl : imageSource
                onStatusChanged: {
                    if( status == Image.Ready ) {
                        if( imageDelegate.enterLeaveState === 2 ) {
                            imageDelegate.enter(imageDelegate.itemX,
                                                imageDelegate.itemY,
                                                imageDelegate.itemW,
                                                imageDelegate.itemH,
                                                imageDelegate.itemSourceW,
                                                imageDelegate.itemSourceH)
                        }
                        else {
                            if( flick.width == 0 && flick.height == 0 && imagePlayer.ready ) {
                                imageDelegate.enterLeaveState = 3
                                flick.width = imageDelegate.width
                                flick.height = imageDelegate.height
                                imageDelegate.ratio = imageDelegate.getImageRatio()
                                bind()
                            }
                        }
                    }
                }
            }
        }

        MouseArea {
            Timer {
                id: timer
                interval: 500
                onTriggered: imageDelegate.clicked()
            }

            anchors.fill: parent

            property real pressedX
            property real pressedY
            property real currentX
            property real currentY
            property bool dragging: false
            // 移动flick content时
            // mouse position接收的mouse坐标会发生偏移
            // 表现为
            // 如果flick x移动了10个像素, 下次mouse x则会-10, 拖动会发生跳动
            property real minusX
            property real minusY


            onPressed: {
                moving = false
                dragging = false
                pressedX = mouseX
                pressedY = mouseY
                currentX = flick.contentX
                currentY = flick.contentY
                minusX = 0
                minusY = 0
            }
            onPositionChanged: {
                var y = (mouseY + minusY) - pressedY
                var x = (mouseX + minusX) - pressedX
                if( moving ) {
                    flick.contentY = currentY - y
                    flick.contentX = currentX - x
                    minusX = x
                    minusY = y
                }
                else {
                    if( y > 20 && flick.atYBeginning ) {
                        moving = true
                        dragging = true
                        flick.interactive = false
                    }
                    else if( (flick.width < flick.contentWidth)
                            && ((x > 0 && flick.atXBeginning) || (x < 0 && flick.atXEnd)) ) {
                        // 放大时
                        // 拖动到x begin 或x end时, 禁止flick滑动, 让list处理滑动切换
                        flick.interactive = false
                    }
                }
            }
            onReleased: {
                if( moving ) {
                    moving = false
                    leave(delegateIndex)
                }
                flick.interactive = true
            }

            onClicked: {
                if( dragging ) {
                    dragging = false
                    return
                }
                timer.start()
            }
            // 双击放大 缩小
            onDoubleClicked: imageDelegate.resizeFlickContent(mouseX, mouseY)
        }
    }

    MouseArea {
        visible: VideoPlayer.playIndex === photoScan.currentIndex ?
                     false : (fileType === 1 ? ((enterLeaveState === 3) ? !moving : false) : false)
        anchors.centerIn: parent
        width: 50
        height: 50
        Text {
            anchors.fill: parent
            font.family: Config.fontLight
            font.pixelSize: parent.width
            text: "\uf144"
            color: parent.pressed ? "gray" : "white"
        }
        onClicked: {
            imagePlayer.hideTitle()
            VideoPlayer.openStream(filePath, photoScan.currentIndex)
        }
    }

    function getImageRatio() {
        if( imageDelegate.width < 1 || imageDelegate.height < 1 ) {
            return 0
        }

        var ratio = 0
        var w2hRatio = image.sourceSize.width / image.sourceSize.height
        var h2wRatio = image.sourceSize.height / image.sourceSize.width
        if( imageDelegate.landscape ) {

        }
        else {
            if( imageDelegate.w2hRatio < w2hRatio ) {
                ratio = imageDelegate.width / image.sourceSize.width
            }
            else {
                ratio = imageDelegate.height / image.sourceSize.height
            }
        }

        return ratio
    }

    property real itemX: 1
    property real itemY: 1
    property real itemW: 1
    property real itemH: 1
    property real itemSourceW: 1
    property real itemSourceH: 1
    property real contentWidth: 1
    property real contentHeight: 1
    // 0: left
    // 1: leaving
    // 2: entering
    // 3: entered
    property int enterLeaveState: 0

    property real ratio
    property int animationDuration: 200

    function enter(index) {
        var item = imageList.itemRect(index)
//        console.log("enter:", item.x, item.y, item.w, item.h, item.pw, item.ph)
        imageDelegate.itemX = item.x
        imageDelegate.itemY = item.y
        imageDelegate.itemW = item.w
        imageDelegate.itemH = item.h
        imageDelegate.itemSourceW = item.pw
        imageDelegate.itemSourceH = item.ph
        imageDelegate.ratio = imageDelegate.getImageRatio()
        imageDelegate.visible = true
        enterLeaveState = 2
        if( image.status !== Image.Ready ) {
            return
        }
        imageDelegate.contentWidth = image.sourceSize.width * imageDelegate.ratio
        imageDelegate.contentHeight = image.sourceSize.height * imageDelegate.ratio
        enterLeaveAnimation.running = true
    }

    function leave(index) {
        var item = imageList.itemRect(index)
//        console.log("leave:", item.x, item.y, item.w, item.h, item.pw, item.ph)
        imageDelegate.itemX = item.x
        imageDelegate.itemY = item.y
        imageDelegate.itemW = item.w
        imageDelegate.itemH = item.h
        imageDelegate.itemSourceW = item.pw
        imageDelegate.itemSourceH = item.ph
        flick.x = 0 - flick.contentX
        flick.y = 0 - flick.contentY
        flick.contentX = 0
        flick.contentY = 0
        enterLeaveState = 1
        enterLeaveAnimation.running = true
    }

    // 进入时
    // flick x, y, w, h 为列表item的x, y, w, h
    // 放大至x: 0, y:0, w: 窗口宽, h: 窗口高
    // image 宽高w, h 为列表item的image painted w, h
    // 放大至比例宽高
    //
    // 离开时
    //
    ParallelAnimation {
        id: enterLeaveAnimation
        PropertyAnimation {
            target: flick
            property: "x"
            from: enterLeaveState == 1 ? flick.x : itemX
            to: enterLeaveState == 1 ? itemX : 0
            duration: animationDuration
        }
        PropertyAnimation {
            target: flick
            property: "y"
            from: enterLeaveState == 1 ? flick.y : itemY
            to: enterLeaveState == 1 ? itemY : 0
            duration: animationDuration
        }
        PropertyAnimation {
            target: flick
            property: "width"
            from: enterLeaveState == 1 ? flick.contentWidth : itemW
            to: enterLeaveState == 1 ? itemW : imageDelegate.width
            duration: animationDuration
        }
        PropertyAnimation {
            target: flick
            property: "height"
            from: enterLeaveState == 1 ? flick.contentHeight : imageDelegate.itemH
            to: enterLeaveState == 1 ? itemH : imageDelegate.height
            duration: animationDuration
        }
        PropertyAnimation {
            target: image
            property: "width"
            from: enterLeaveState == 1 ? image.paintedWidth : imageDelegate.itemSourceW
            to: enterLeaveState == 1 ? itemSourceW : imageDelegate.contentWidth
            duration: animationDuration
        }
        PropertyAnimation {
            target: image
            property: "height"
            from: enterLeaveState == 1 ? image.paintedHeight : imageDelegate.itemSourceH
            to: enterLeaveState == 1 ?  itemSourceH : imageDelegate.contentHeight
            duration: animationDuration
        }
        onRunningChanged: {
            if( !running ) {
                if( enterLeaveState === 2 ) {
                    enterLeaveState = 3
                    image.bind()
                }
                else if( enterLeaveState == 1 ) {
                    enterLeaveState = 0
                }
            }
        }
    }


    // 图片双击放大缩小时, 判断是否为有效点击
    // 有效点击
    // mouseX >= image.x && mouseX <= (image.x + image.width)
    // mouseY >= image.y && mouseY <= (image.y + image.height)
    function isValidClicked(mouseX, mouseY) {
        if( (mouseX >= image.x && mouseX <= (image.x + image.width))
                && (mouseY >= image.y && mouseY <= (image.y + image.height)) ) {
            return true
        }
        return false
    }

    function resizeFlickContent(mouseX, mouseY) {
        if( enterLeaveAnimation.running ) {
            return
        }

        if( !isValidClicked(mouseX, mouseY) ) {
            return
        }

        timer.stop()
        if( flick.contentWidth > flick.width
                || flick.contentHeight > flick.height ) {
            resizeAnimation.resize(flick.x, flick.y, flick.width, flick.height)
        }
        else {
            var ratio = imageDelegate.landscape ?
                        flick.width / image.sourceSize.width : flick.height / image.sourceSize.height
            var w = image.sourceSize.width * ratio
            var h = image.sourceSize.height * ratio

            var mouseRatio = w / flick.width

            var cx = w / 2
            var cy = h / 2

            var xMax = w - flick.width
            var x = (mouseX * mouseRatio) - cx
            x = x < 0 ? 0 : (x > xMax ? xMax : x)
            console.log(mouseX * mouseRatio, cx)

            var yMax = h - flick.height
            var y = cy + (mouseY - cy)
            y = y < 0 ? 0 : (y > yMax ? yMax : y)
            resizeAnimation.resize(x, y, w, h)
        }
    }

    ParallelAnimation {
        id: resizeAnimation

        function resize(x, y, w, h) {
            resizeAnimation.toX = x
            resizeAnimation.toY = y
            resizeAnimation.toW = w
            resizeAnimation.toH = h
            resizeAnimation.running = true
        }

        property real toX
        property real toY
        property real toW
        property real toH
        PropertyAnimation {
            target: flick
            property: "contentWidth"
            from: flick.contentWidth
            to: resizeAnimation.toW
            duration: imageDelegate.animationDuration
            easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: flick
            property: "contentHeight"
            from: flick.contentHeight
            to: resizeAnimation.toH
            duration: imageDelegate.animationDuration
            easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: flick
            property: "contentX"
            from: flick.contentX
            to: resizeAnimation.toX
            duration: imageDelegate.animationDuration
            easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: flick
            property: "contentY"
            from: flick.contentY
            to: resizeAnimation.toY
            duration: imageDelegate.animationDuration
            easing.type: Easing.OutCubic
        }
    }
}
