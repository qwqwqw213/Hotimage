import QtQuick 2.14
import QtQuick.Controls 2.12
import Qt.labs.folderlistmodel 2.14

//import Custom.ImagePaintView 1.1

Item {
    id: imagePlayer

    onVisibleChanged: {
        if( visible === false ) {
            toolbar.opacity = 1
        }
    }

    function hideTitle() {
        if( imagePlayer.visible ) {
            toolbar.opacity = 0
        }
    }

    function autoTitle() {
        if( toolbar.opacity > 0 ) {
            toolbar.opacity = 0
        }
        else {
            toolbar.opacity = 1
        }
    }

    property bool isShow: false

    /*
     *  缩放动画需要缩放 图像外框 和 图像
     *  图像外框 需要 8 个参数
     *  起始坐标 x, y (进入时, GridView的item坐标, 退出时: 屏幕左上角坐标)
     *  起始宽高 w, h (进入时: GridView的item宽高, 退出时: 屏幕宽高)
     *  终点坐标 x, y (进入时: 屏幕左上角坐标, 退出时: GridView的item坐标)
     *  终点宽高 w, h (进入时: 屏幕宽高, 退出时: GridView的Item 宽高)
     *  图像 需要 4 个参数
     *  图像起始宽高(进入时: GridView的item图形paintedWidth, Height, 退出时: 当前paintedWidth, Height)
     *  图像终点宽高(进入时: 当前paintedWidth, Height, 退出时: GridView的item图形paintedWidth, Height)
     */
    function zoom(index, path, x, y, w, h, pw, ph) {
        photoScan.interactive = false
        imageZoomItem.visible = false
        imageZoomItem.fromX = x
        imageZoomItem.fromY = y
        imageZoomItem.fromW = w
        imageZoomItem.fromH = h
        imageZoomItem.toX = imagePlayer.x
        imageZoomItem.toY = imagePlayer.y
        imageZoomItem.toW = imagePlayer.width
        imageZoomItem.toH = imagePlayer.height
        imageZoomItem.isZoom = true

        imageZoomItemSource.fromW = pw
        imageZoomItemSource.fromH = ph
        imageZoomItemSource.source = path

        photoScan.currentIndex = index
        photoScan.positionViewAtIndex(index, ListView.Beginning)
        miniPhtotList.positionViewAtIndex(index, ListView.Center)
    }

    function quit() {
        photoScan.interactive = false
        imageZoomItem.isZoom = false

        var scanItem = photoScan.currentItem
        imageZoomItem.fromX = 0 - scanItem.contentX
        imageZoomItem.fromY = 0 - scanItem.contentY
        imageZoomItem.fromW = scanItem.contentW
        imageZoomItem.fromH = scanItem.contentH

        var listItemRect = imageListView.itemRect(photoScan.currentIndex)
        imageZoomItem.toX = listItemRect.x
        imageZoomItem.toY = listItemRect.y
        imageZoomItem.toW = listItemRect.w
        imageZoomItem.toH = listItemRect.h

        imageZoomItemSource.fromW = scanItem.paintW
        imageZoomItemSource.fromH = scanItem.paintH
        if( imageZoomItemSource.source === scanItem.source ) {
            imageZoomItemSource.init()
        }
        else {
            imageZoomItemSource.source = scanItem.source
        }
    }

    Item {
        id: imageZoomItem

        property bool isZoom
        property real fromX
        property real fromY
        property real fromW
        property real fromH
        property real toX
        property real toY
        property real toW
        property real toH

        z: 2
        visible: false
        clip: true

        Image {
            id: imageZoomItemSource
            asynchronous: true
            anchors.centerIn: parent


            property real fromW
            property real fromH
            property real toW
            property real toH

            function init() {
                imageZoomItem.visible = true
                var screenRatio = imageZoomItem.toW / imageZoomItem.toH
                var imageRatio = imageZoomItemSource.implicitWidth / imageZoomItemSource.implicitHeight

                if( imageZoomItem.isZoom )
                {
                    var toScale
                    if( Config.isLandscape )
                    {
                        toScale = imageRatio > 1 ?
                                    (screenRatio > imageRatio ?
                                         imageZoomItem.toH / imageZoomItemSource.implicitHeight
                                       : imageZoomItem.toW / imageZoomItemSource.implicitWidth)
                                  : imageZoomItem.toH / imageZoomItemSource.implicitHeight
                        toW = imageZoomItemSource.implicitWidth * toScale
                        toH = imageZoomItemSource.implicitHeight * toScale
                    }
                    else
                    {
                        toScale = imageRatio > 1 ?
                                    imageZoomItem.toW / imageZoomItemSource.implicitWidth
                                  : (imageRatio > screenRatio ?
                                         imageZoomItem.toW / imageZoomItemSource.implicitWidth
                                       : imageZoomItem.toH / imageZoomItemSource.implicitHeight)
                        toW = imageZoomItemSource.implicitWidth * toScale
                        toH = imageZoomItemSource.implicitHeight * toScale
                    }
                }
                else
                {
                    var fromScale = imageRatio > 1 ?
                                imageZoomItem.toH / imageZoomItemSource.implicitHeight
                              : imageZoomItem.toW / imageZoomItemSource.implicitWidth
                    toW = imageZoomItemSource.implicitWidth * fromScale
                    toH = imageZoomItemSource.implicitHeight * fromScale
                }

                imageZoomItemAnimation.running = true
            }

            onStatusChanged: {
                if( imageZoomItemSource.status == Image.Ready )
                {
                    init()
                }
            }
        }

        ParallelAnimation {
            id: imageZoomItemAnimation
            property int duration: 200
            PropertyAnimation {
                target: imageZoomItem
                properties: "x"
                from: imageZoomItem.fromX
                to: imageZoomItem.toX
                duration: imageZoomItemAnimation.duration
                easing.type: Easing.InQuad
            }
            PropertyAnimation {
                target: imageZoomItem
                properties: "y"
                from: imageZoomItem.fromY
                to: imageZoomItem.toY
                duration: imageZoomItemAnimation.duration
                easing.type: Easing.InQuad
            }
            PropertyAnimation {
                target: imageZoomItem
                properties: "width"
                from: imageZoomItem.fromW
                to: imageZoomItem.toW
                duration: imageZoomItemAnimation.duration
                easing.type: Easing.InQuad
            }
            PropertyAnimation {
                target: imageZoomItem
                properties: "height"
                from: imageZoomItem.fromH
                to: imageZoomItem.toH
                duration: imageZoomItemAnimation.duration
                easing.type: Easing.InQuad
            }
            PropertyAnimation {
                target: imageZoomItemSource
                properties: "width"
                from: imageZoomItemSource.fromW
                to: imageZoomItemSource.toW
                duration: imageZoomItemAnimation.duration
                easing.type: Easing.InQuad
            }
            PropertyAnimation {
                target: imageZoomItemSource
                properties: "height"
                from: imageZoomItemSource.fromH
                to: imageZoomItemSource.toH
                duration: imageZoomItemAnimation.duration
                easing.type: Easing.InQuad
            }
            onStarted: {
                if( imageZoomItem.isZoom )
                {
                    imageListView.interactive = false
                    imagePlayer.visible = true
                    imagePlayer.isShow = true
                    childOpacity = 1
                }
                else
                {
                    photoScan.visible = false
                    childOpacity = 0
                }
            }

            onFinished: {
                if( imageZoomItem.isZoom )
                {
                    photoScan.interactive = true
                    photoScan.visible = true
                }
                else
                {
                    imagePlayer.isShow = false
                    imageListView.interactive = true
                    imageZoomItemSource.source = ""
                    imagePlayer.visible = false
                    var scanItem = photoScan.currentItem
                    scanItem.contentX = 0
                    scanItem.contentY = 0
                    scanItem.contentW = photoScan.width
                    scanItem.contentH = photoScan.height
                }
            }
        }
    }

    property real childOpacity: 0
    Behavior on childOpacity {
        NumberAnimation { duration: 200 }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: childOpacity
    }

    ListView {
        id: photoScan
        anchors.fill: parent
//        model: scanModel
        model: ImageModel
        cacheBuffer: 5
        clip: true
        z: 1
        interactive: VideoPlayer.playing > 0 ? false : true
        spacing: 20
        highlightRangeMode: ListView.StrictlyEnforceRange
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
//        visible: imagePlayer.state == "show"
        visible: false
        onVisibleChanged: {
            if( visible ) {
                timer.start()
            }
        }

        Timer {
            id: timer
            interval: 60
            onTriggered: {
                imageZoomItem.visible = false
//                imageZoomItemSource.source = ""
            }
        }

        delegate: AlbumScanDelegate {
            width: photoScan.width
            height: photoScan.height
            onDragOut: {
//                console.log(p, toolbar.barY)
//                if( toolbar.barY === 60 ) {
//                    toolbar.barY = p * 60
//                }
            }
        }

        property real maxContentX: width * (ImageModel.rowCount() - 1)
        onMovementStarted: {

        }

        onMovementEnded: {
//            imagePlayer.currentIndexChanged(ImageModel.currentIndex)
        }

        onCurrentIndexChanged: {
//            console.log("index changed", currentIndex)
            ImageModel.currentIndex = currentIndex
            VideoPlayer.closeStream()
//            if( !miniPhtotList.moving ) {
//                miniPhtotList.positionViewAtIndex(ImageModel.currentIndex,  ListView.Center)
//            }
        }

        onContentXChanged: {
            /*
             *  photoScan 滑动时
             *  miniPhtotList 跟着滑动
             *  当miniPhtotList 发生点击事件时
             *  miniPhtotList.Behavior会被设置为true
             *  因为点击事件会改变photoScan的contentX
             *  所以miniPhtotList会做出移动动画
             */
            miniPhtotList.contentX = (contentX - currentIndex * spacing)
                    / photoScan.width
                    * miniPhtotList.cellWidth
                    + miniPhtotList.originX
                    - miniPhtotList.leftMargin
        }
    }

    Item {
        id: toolbar
        anchors.fill: parent
        z: 5

        Behavior on opacity {
            OpacityAnimator { duration: 200 }
        }

        // top bar
        Rectangle {
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
                        imagePlayer.quit()
                        VideoPlayer.closeStream()
                    }
                }

                Text {
                    anchors.left: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: ImageModel.name
                    color: "white"
                }
            }
        }

        // bottom bar
        Rectangle {
            width: parent.width
            height: 60 + Config.bottomMargin
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            color: "#2f4f4f"

            MouseArea {
                anchors.fill: parent
            }

            /*
             *  底部 图片列表 缩略图
             */
            GridView {
                id: miniPhtotList
                width: parent.width
                height: parent.height - Config.bottomMargin
                anchors.top: parent.top
                model: ImageModel
                cellWidth: width > height ? height : width
                cellHeight: cellWidth

                flow: GridView.FlowTopToBottom
                layoutDirection: GridView.LeftToRight

                leftMargin: width / 2 - cellWidth / 2
                rightMargin: width / 2 - cellWidth / 2

                Behavior on contentX {
                    id: moveBehavior
                    enabled: false
                    NumberAnimation {
                        duration: 200
                        onRunningChanged: {
                            if( !running ) {
                                moveBehavior.enabled = false
                            }
                        }
                    }
                }

                onMovementEnded: {
                    var temp = contentX % (cellWidth / 2)
                    console.log("temp:", temp, contentX, leftMargin)
                }

                delegate: Rectangle {
                    id: itembg
                    width: miniPhtotList.cellWidth
                    height: miniPhtotList.cellHeight
                    color: "transparent"
                    Image {
                        asynchronous: true
                        id: photo
    //                    source: fileType == 0 ? path : ""
                        source: path
                        anchors.centerIn: parent
                        width: parent.width - 2
                        height: parent.height - 2

                        sourceSize: Qt.size(parent.width, parent.height)
                        smooth: false
                        fillMode: Image.PreserveAspectCrop
                        cache: false

                        Text {
                            visible: fileType === 1
                            anchors.centerIn: parent
                            font.family: "FontAwesome"
                            font.pixelSize: parent.width * 0.35
                            text: "\uf144"
                            color: "white"
                        }
                    }

                    MouseArea{
                        id: mouseArea
                        anchors.fill: parent
                        onClicked: {
                            console.log("mini list clicked index:", index)
                            moveBehavior.enabled = true
//                            miniPhtotList.positionViewAtIndex(index, ListView.Center)
//                            miniPhtotList.contentX = 0
                            photoScan.positionViewAtIndex(index, ListView.Beginning)
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: videoViewLoader
        property real progressBarOpacity: 1
        active: VideoPlayer.playing > 0 ? true : false
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
}
