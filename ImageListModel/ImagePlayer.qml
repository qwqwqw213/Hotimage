import QtQuick 2.14
import QtQuick.Controls 2.12
import Qt.labs.folderlistmodel 2.14

import "../BackTitle"

Item {
    id: imagePlayer

    function open(index) {
        photoScan.currentIndex = index
        photoScan.positionViewAtIndex(index, ListView.Beginning)
        var item = photoScan.itemAtIndex(index)
        item.enter(index)
        imagePlayer.visible = true
    }
    function quit() {
        var item = photoScan.itemAtIndex(photoScan.currentIndex)
        item.leave(photoScan.currentIndex)
    }

    onVisibleChanged: {
        if( stackViewMouseArea ) {
            stackViewMouseArea.enabled = !visible
        }
    }

    function hideTitle() {
        if( imagePlayer.visible ) {
            toolbar.opacity = 0
        }
    }

    function autoTitle() {
//        if( toolbar.opacity > 0 ) {
//            toolbar.opacity = 0
//        }
//        else {
//            toolbar.visible = true
//            toolbar.opacity = 1
//        }
        if( toolbar.visible ) {
            toolbar.opacity = 0
        }
        else {
            toolbar.visible = true
            toolbar.opacity = 1
        }
    }

    property real childOpacity: 1

    Rectangle {
        id: background
        anchors.fill: parent
        color: "black"
        opacity: childOpacity
    }

    property bool ready: false
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

        delegate: ImageDelegate {
            width: photoScan.width
            height: photoScan.height
            imageSource: path
            delegateIndex: index
            onEnterLeaveStateChanged: {
                if( index === photoScan.currentIndex ) {
                    if( enterLeaveState == 0 ) {
                        imagePlayer.visible = false
                    }
                    else if( enterLeaveState == 1 ) {
                        imagePlayer.ready = false
                    }
                    else if( enterLeaveState == 3 ) {
                        imagePlayer.ready = true
                    }
                }
            }
            onClicked: {
                autoTitle()
            }
            onEnterLeaveRatioChanged: {
                if( index === photoScan.currentIndex ) {
                    childOpacity = enterLeaveRatio
                }
            }
            onMovingChanged: {
                if( index === photoScan.currentIndex ) {
                    photoScan.interactive = !moving
                }
            }
        }

        property real maxContentX: width * (ImageModel.rowCount() - 1)

        onCurrentIndexChanged: {
            ImageModel.currentIndex = currentIndex
            VideoPlayer.closeStream()
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

        opacity: childOpacity

        Behavior on opacity {
            enabled: ready
            NumberAnimation {
                duration: 200
                onRunningChanged: {
                    if( !running ) {
                        if( toolbar.opacity < 1 ) {
                            toolbar.visible = false
                        }
                    }
                }
            }
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
                font.family: Config.fontRegular
                font.pixelSize: width * 0.75
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
                            font.family: Config.fontLight
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
        property real progressBarOpacity: 1
        active: VideoPlayer.isValid
        anchors.fill: parent
        sourceComponent: videoToolComponent
        z: 5

        Component {
            id: videoToolComponent
            Item {
                Behavior on opacity {
                    OpacityAnimator {
                        duration: 150
                    }
                }
                anchors.fill: parent
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if( parent.opacity > 0 ) {
                            parent.opacity = 0
                        }
                        else {
                            parent.opacity = 1
                        }
                    }
                }

                // 视频顶部按钮
                Rectangle {
                    id: btnVideoQuit
                    width: 50
                    height: 50
                    anchors.left: progressBarItem.left
                    anchors.top: parent.top
                    anchors.topMargin: 10 + Config.topMargin
                    radius: 10
                    color: "#D0505050"

                    Text {
                        id: btnVideoQuitIcon
                        font.family: Config.fontLight
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
                            autoTitle()
                        }
                    }
                }

                VideoProgressbar {
                    id: progressBarItem
                    width: parent.width * 0.85
                    height: Config.isLandscape ? 60 : 120
                    x: (parent.width - width) / 2.0
                    y: parent.height - height - 10 - Config.bottomMargin
                }
            }
        }
    }
}
