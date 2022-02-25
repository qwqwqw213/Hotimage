import QtQuick 2.11

Item {
    id: wrapper

    signal itemClicked
    signal dragOut(var p)

    property alias contentX: flick.contentX
    property alias contentY: flick.contentY
    property alias contentW: flick.contentWidth
    property alias contentH: flick.contentHeight
    property alias paintW: image.paintedWidth
    property alias paintH: image.paintedHeight
    property alias source: image.source

    property alias parentW: wrapper.width
    property alias parentH: wrapper.height

    Flickable {
        id: flick
//        anchors.fill: parent
        width: parent.width
        height: parent.height
        clip: true

        contentWidth: wrapper.width
        contentHeight: wrapper.height

        onContentXChanged: {
          imagePlayer.hideTitle()
        }

        onContentYChanged: {
          imagePlayer.hideTitle()
        }

        PinchArea {
            id: pinchArea
            width: flick.contentWidth
            height: flick.contentHeight
            enabled: fileType === 0

            property real pressedWidth
            property real pressedHeight
            property real minScaleWidth: wrapper.width * 0.85
            property real minScaleHeight: wrapper.height * 0.85

            onPinchStarted: {
                pressedWidth = flick.contentWidth
                pressedHeight = flick.contentHeight
                resizeAnimation.running = false
                imagePlayer.hideTitle()
            }

            onPinchUpdated: {
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
                // Move its content within bounds.
                if( flick.contentWidth < wrapper.width || flick.contentHeight < wrapper.height ) {
                    resizeAnimation.run(0, 0, wrapper.width, wrapper.height)
                }
                else {
                    flick.returnToBounds()
                }
            }

            Image {
                id: image
                width: flick.contentWidth - (flick.contentWidth * (0.4 * dragScale))
                height: flick.contentHeight - (flick.contentHeight * (0.4 * dragScale))

//                source: path
                source: VideoPlayer.playing > 0 ?
                            ((VideoPlayer.playIndex === index) ? VideoPlayer.frameUrl : path)
                          : path

                asynchronous: VideoPlayer.playing > 0 ?
                                  ((VideoPlayer.playIndex === index) ? false : true)
                                : true

                fillMode: Image.PreserveAspectFit
                cache: false

                property real dragScale: Math.abs(y / wrapper.height)
//                onYChanged: {
//                    imagePlayer.childOpacity = 1 - Math.abs(y / wrapper.height)
//                }
            }


//            MouseArea {
//                id: mouseArea
//                Timer {
//                    id: timer
//                    interval: 100
//                    onTriggered: {
//                        imagePlayer.autoTitle()
//                    }
//                }

//                function isValidClicked(x, y) {
//                    var left = (image.width - image.paintedWidth) / 2
//                    var right = left + image.paintedWidth
//                    var top = (image.height - image.paintedHeight) / 2
//                    var bottom = top + image.paintedHeight

//                    if( x >= left && x <= right
//                            && y >= top && y <= bottom ) {
//                        return {
//                            res: true,
//                            x: x - image.width / 2,
//                            y: y - image.height / 2
//                        }
//                    }
//                    else {
//                        return {
//                            res: false,
//                            x: 0,
//                            y: 0
//                        }
//                    }
//                }

//                anchors.fill: parent

//                property real pressedX
//                property real pressedY
//                property real currentX
//                property real currentY
//                property bool dragFlag: false


//                onPressed: {
//                    pressedX = mouseX
//                    pressedY = mouseY
//                    currentX = image.x
//                    currentY = image.y
//                }

//                onPositionChanged: {
//                    if( dragFlag ) {
//                        /*
//                         * 下拽退出时
//                         * 图片宽度会逐渐缩小, 位置会逐渐偏左
//                         * 所以需要用(flick.contentWidth - image.width) / 2 做居中处理
//                         */
//                        image.x = mouseX - pressedX + currentX + (flick.contentWidth - image.width) / 2
//                        var p = (mouseY - pressedY + currentY) / wrapper.height
//                        image.y = p * image.height
//                        wrapper.dragOut(Math.abs(p))
//                    }
//                    else if( flick.width < flick.contentWidth
//                            && flick.height < flick.contentHeight
//                            && flick.atYBeginning ) {
//                        dragFlag = true
//                        photoScan.interactive = false
//                        flick.interactive = false
//                    }
//                    else {
//                        var y = mouseY - pressedY;
//                        if( y > 30 && flick.atYBeginning ) {
//                            dragFlag = true
//                            photoScan.interactive = false
//                            flick.interactive = false
//                        }
//                    }
//                }

//                onReleased: {
//                    if( dragFlag ) {
//                        dragFlag = false
//                        flick.interactive = true
//                        photoScan.interactive = true
//                        image.x = 0
//                        image.y = 0
//                        var rect = imageListView.itemRect(index);
//                    }
//                }


//                onDoubleClicked: {
//                    var ret = isValidClicked(mouseX, mouseY)
//                    if( ret.res && fileType === 0 )
//                    {
//                        imagePlayer.hideTitle()

//                        if( flick.contentWidth > wrapper.width
//                                || flick.contentHeight > wrapper.height ) {
//                            // zoom out
//                            resizeAnimation.run(0, 0, wrapper.width, wrapper.height)
//                        }
//                        else {
//                            // zoom in
//                            var w = wrapper.width * 3
//                            var h = wrapper.height * 3
//                            var cx = (w - wrapper.width) / 2
//                            var cy = (h - wrapper.height) / 2
//                            var x = cx + ret.x * 3
//                            if( x < 0 ) {
//                                x = 0
//                            }
//                            if( x > (w - wrapper.width) ) {
//                                x = w - wrapper.width
//                            }

//                            var y = cy + ret.y * 3
//                            if( y < 0 ) {
//                                y = 0;
//                            }
//                            if( y > (h - wrapper.height) ) {
//                                y = wrapper.height
//                            }

//                            resizeAnimation.run(x, y, w, h)
//                        }

//                        timer.stop()
//                    }
//                }
//                onClicked: {
//                    wrapper.itemClicked()
//                    if( fileType === 1 )
//                    {
//                        if( VideoPlayer.playing > 0 )
//                        {
//                            if( videoViewLoader.progressBarOpacity > 0 ) {
//                                videoViewLoader.progressBarOpacity = 0
//                            }
//                            else {
//                                videoViewLoader.progressBarOpacity = 1
//                            }
//                        }
//                        else {
//                            timer.start();
//                        }
//                    }
//                    else {
//                        timer.start();
//                    }
//                }
//            }

            Text {
                id: btnVideoPlay
                visible: fileType === 1 ?
                             (VideoPlayer.playing > 0 ? (VideoPlayer.playIndex === index ? false : true) : true)
                           : false
                anchors.centerIn: parent
                font.family: "FontAwesome"
                font.pixelSize: parent.width > parent.height ? parent.width * 0.15 : parent.height * 0.15
                text: "\uf144"
                color: btnVideoPlayArea.pressed ? "#a0a0a0" : "white"

                MouseArea {
                    id: btnVideoPlayArea
                    width: btnVideoPlay.contentWidth
                    height: btnVideoPlay.contentHeight
                    anchors.centerIn: parent
                    onClicked: {
                        if( VideoPlayer.playing < 1 ) {
                            console.log("video play index:", index)
  //                          imagePaintView.openStream(filePath, image.paintedWidth, image.paintedHeight)
                            VideoPlayer.openStream(filePath, image.paintedWidth, image.paintedHeight, index)
                            imagePlayer.hideTitle()
//                            videoViewLoader.progressBarOpacity = 1
                        }
                    }
                }
            }

            /*
            // VideoView
            Loader {
                id: videoViewLoader
                property real progressBarOpacity: 1

                active: VideoPlayer.playing > 0 ?
                            ((VideoPlayer.playIndex === index) ? true : false) : false
//                active: false
                anchors.centerIn: parent
                sourceComponent: videoView

                Component {
                    id: videoView
                    Image {
                        width: flick.contentWidth - 10
                        height: flick.contentHeight - 10
                        source: VideoPlayer.playIndex === index ? VideoPlayer.frameUrl : ""
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit

                        // 视频顶部按钮
                        Rectangle {
                            id: btnVideoQuit
                            width: 50
                            height: 50
                            anchors.left: progressBarItem.left
                            anchors.top: parent.top
                            anchors.topMargin: 10
                            radius: 10
                            color: "#D0505050"
                            opacity: videoViewLoader.progressBarOpacity

                            Text {
                                id: btnVideoQuitIcon
                                font.family: "FontAwesome"
                                font.pixelSize: parent.height * 0.75
                                color: btnVideoQuitArea.pressed ? "#f0f0f0" : "white"
                                text: "\uf00d"
                                anchors.centerIn: parent
                                Behavior on scale {
                                    NumberAnimation {
                                        duration: 200
                                    }
                                }
                            }

                            MouseArea {
                                id: btnVideoQuitArea
                                anchors.fill: parent
                                onPressed: {
                                    btnVideoQuitIcon.scale = 0.75
                                }
                                onReleased: {
                                    btnVideoQuitIcon.scale = 1
                                }
                                onClicked: {
                                    // close video stream
                                    VideoPlayer.closeStream()
                                }
                            }
                        }

                        VideoProgressbar {
                            id: progressBarItem
                            width: parent.width * 0.85
                            height: Config.isLandscape ? 60 : 120
                            x: (parent.width - width) / 2.0
                            y: parent.height - height - 10
                            opacity: videoViewLoader.progressBarOpacity
                        }
                    }
                }
            }
            */

            ParallelAnimation {
                id: resizeAnimation
                running: false
                loops: 1

                property real to_x
                property real to_y
                property real to_w
                property real to_h
                function run(x, y, w, h) {
                    to_x = x
                    to_y = y
                    to_w = w
                    to_h = h
                    running = true
                }

                PropertyAnimation {
                    target: flick
                    property: "contentWidth"
                    from: flick.contentWidth
                    to: resizeAnimation.to_w
                    duration: 200
                    easing.type: Easing.OutCubic
                }
                PropertyAnimation {
                    target: flick
                    property: "contentHeight"
                    from: flick.contentHeight
                    to: resizeAnimation.to_h
                    duration: 200
                    easing.type: Easing.OutCubic
                }
                PropertyAnimation {
                    target: flick
                    property: "contentX"
                    from: flick.contentX
                    to: resizeAnimation.to_x
                    duration: 200
                    easing.type: Easing.OutCubic
                }
                PropertyAnimation {
                    target: flick
                    property: "contentY"
                    from: flick.contentY
                    to: resizeAnimation.to_y
                    duration: 200
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        property real pressedX
        property real pressedY
        property real currentX
        property real currentY
        property bool dragFlag: false
        enabled: false


        onPressed: {
            console.log("pressed")
            pressedX = mouseX
            pressedY = mouseY
            currentX = image.x
            currentY = image.y
        }

        onPositionChanged: {
            if( dragFlag ) {
                /*
                 * 下拽退出时
                 * 图片宽度会逐渐缩小, 位置会逐渐偏左
                 * 所以需要用(flick.contentWidth - image.width) / 2 做居中处理
                 */
                image.x = mouseX - pressedX + currentX + (flick.contentWidth - image.width) / 2
                var p = (mouseY - pressedY + currentY) / wrapper.height
                image.y = p * image.height
                wrapper.dragOut(Math.abs(p))
            }
            else if( flick.width < flick.contentWidth
                    && flick.height < flick.contentHeight
                    && flick.atYBeginning ) {
                dragFlag = true
                photoScan.interactive = false
                flick.interactive = false
            }
            else {
                var y = mouseY - pressedY;
                if( y > 30 && flick.atYBeginning ) {
                    dragFlag = true
                    photoScan.interactive = false
                    flick.interactive = false
                }
            }
        }

        onReleased: {
            if( dragFlag ) {
                dragFlag = false
                flick.interactive = true
                photoScan.interactive = true
                image.x = 0
                image.y = 0
                var rect = imageListView.itemRect(index);
            }
        }
    }

    ParallelAnimation {
        id: inoutAnimation

        property int duration: 200

//        PropertyAnimation {
//            target: flick
//            properties: "x"
//            from: imageZoomItem.fromX
//            to: imageZoomItem.toX
//            duration: imageZoomItemAnimation.duration
//            easing.type: Easing.InQuad
//        }
//        PropertyAnimation {
//            target: flick
//            properties: "y"
//            from: imageZoomItem.fromY
//            to: imageZoomItem.toY
//            duration: imageZoomItemAnimation.duration
//            easing.type: Easing.InQuad
//        }
//        PropertyAnimation {
//            target: flick
//            properties: "width"
//            from: imageZoomItem.fromW
//            to: imageZoomItem.toW
//            duration: imageZoomItemAnimation.duration
//            easing.type: Easing.InQuad
//        }
//        PropertyAnimation {
//            target: flick
//            properties: "height"
//            from: imageZoomItem.fromH
//            to: imageZoomItem.toH
//            duration: imageZoomItemAnimation.duration
//            easing.type: Easing.InQuad
//        }
//        PropertyAnimation {
//            target: image
//            properties: "width"
//            from: imageZoomItemSource.fromW
//            to: imageZoomItemSource.toW
//            duration: imageZoomItemAnimation.duration
//            easing.type: Easing.InQuad
//        }
//        PropertyAnimation {
//            target: image
//            properties: "height"
//            from: imageZoomItemSource.fromH
//            to: imageZoomItemSource.toH
//            duration: imageZoomItemAnimation.duration
//            easing.type: Easing.InQuad
//        }
//        onStarted: {

//        }

//        onFinished: {

//        }
    }
}
