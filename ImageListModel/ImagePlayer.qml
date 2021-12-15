import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.4
import Qt.labs.folderlistmodel 2.14

//import Custom.ImagePaintView 1.1

Rectangle{
    id: imagePlayer
//    property alias photoScanModel: photoScan.model
    anchors.fill: parent
    color: "black"

    function hideTitle() {
        if( title.state !== "hide" ) {
            title.state = "hide"
            bottom.state = "hide"
        }
    }

    function autoTitle(state) {
        if( title.state === "show" ) {
            title.state = "hide"
            bottom.state = "hide"
        }
        else {
            title.state = "show"
            bottom.state = "show"
        }
    }

    function show(index, x, y, w, h, path) {
        photoScan.currentIndex = index
        photoScan.positionViewAtIndex(index, ListView.Beginning)
        miniPhtotList.positionViewAtIndex(index, ListView.Center)
        visible = true
//        zoomInRect.show(x, y, w, h, path)
        state = "show"
    }

    ListView {
        id: photoScan
        anchors.fill: parent
//        model: scanModel
        model: ImageModel
        cacheBuffer: 5
        clip: true
        z:1
//        maximumFlickVelocity:7000  //设置滑动的最大速度
        highlightRangeMode: ListView.StrictlyEnforceRange
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        visible: imagePlayer.state == "show"

        delegate: AlbumScanDelegate {
            width: photoScan.width
            height: photoScan.height
        }

        property real maxContentX: width * (ImageModel.rowCount() - 1)
        onMovementStarted: {

        }

        onMovementEnded: {
            imageList.positionViewAtIndex(ImageModel.currentIndex,  ListView.End)
        }

        onCurrentIndexChanged: {
            console.log("index changed", currentIndex)
            ImageModel.currentIndex = currentIndex
//            imagePaintView.closeStream()
            VideoPlayer.closeStream()
            if( !miniPhtotList.moving ) {
                miniPhtotList.positionViewAtIndex(ImageModel.currentIndex,  ListView.Center)
            }
        }

        onContentXChanged: {

            if( atXBeginning < 0 ) {
//                photoList.contentX = contentX
            }
            else if( atXEnd ) {
//                photoList.contentX = photoList.maxContentX + contentX - maxContentX
            }
            else {

            }
        }

//        Component.onCompleted: positionViewAtIndex(0, ListView.Beginning)
    }

    /*
//    ImagePaintView {
//        id: imagePaintView
//        x: photoScan.currentIndex * imagePlayer.width - photoScan.contentX
//        width: photoScan.width
//        height: photoScan.height
//        z: 1
//        visible: playing
    Image {
        x: photoScan.currentIndex * imagePlayer.width - photoScan.contentX
//        width: photoScan.width
        width: VideoPlayer.imageWidth
//        height: photoScan.height
        height: VideoPlayer.imageHeight
        z: 1
        visible: VideoPlayer.playing
        source: VideoPlayer.playing ? VideoPlayer.frameUrl : ""

        // 滚动条
        Rectangle {
            id: progressBar
            width: parent.width * 0.85
            height: 60
            x: (parent.width - width) / 2.0
            y: parent.height - height - 10
            radius: 10
            color: "#D0505050"

//            opacity: imagePaintView.playing ? 1 : 0
            opacity: VideoPlayer.playing ? 1 : 0

            Behavior on opacity {
                OpacityAnimator {
                    duration: 200
                }
            }

            MouseArea {
                anchors.fill: parent
            }

            // 暂停按钮
            Text {
                id: btnPause
                text: "\uf28b"
                font.family: "FontAwesome"
                font.pixelSize: parent.height * 0.75
                anchors.left: parent.left
                anchors.leftMargin: contentWidth / 2
                anchors.verticalCenter: parent.verticalCenter
                color: btnPauseArea.pressed ? "#a0a0a0" : "white"
                MouseArea {
                    id: btnPauseArea
                    anchors.fill: parent
                    onClicked: {
//                        console.log("video pause")
//                        imagePaintView.closeStream()
                        VideoPlayer.closeStream()
                        imagePlayer.autoTitle()
                    }
                }
            }

            // 进度条底条
            Rectangle {
                width: parent.width * 0.55
                height: 6
                x: parent.width - parent.width * 0.15 - width
                anchors.verticalCenter: parent.verticalCenter
                color: "#505050"
                radius: 10

                Text {
                    id: currentTime
                    anchors.right: parent.left
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
//                    text: imagePaintView.currentTime
                    text: VideoPlayer.currentTime
                    font.pixelSize: parent.height * 2.5
                    color: "white"
                }

                Text {
                    id: totalTime
                    anchors.left: parent.right
                    anchors.leftMargin:  10
                    anchors.verticalCenter: parent.verticalCenter
//                    text: imagePaintView.totalTime
                    text: VideoPlayer.totalTime
                    font.pixelSize: parent.height * 2.5
                    color: "white"
                }

                //  进度条
                Rectangle {
                    id: bar
//                    width: parent.width * imagePaintView.progress
                    width: parent.width * VideoPlayer.progress
                    height: parent.height
                    color: "white"
                    radius: 10
                }

                // 进度条球
                Rectangle {
                    width: parent.height + 4
                    height: width
                    x: bar.width - width / 2
                    radius: 100
                    color: "white"
                    anchors.verticalCenter: parent.verticalCenter
                    border.width: 1
                    border.color: "#a0a0a0"
                }

                MouseArea {
                    property real pressW
                    property real pressX
                    width: parent.width
                    height: parent.height * 6
                    enabled: false
                    anchors.verticalCenter: parent.verticalCenter
                    onPressed: {
                        pressX = mouseX
                        bar.width = pressX
                        pressW = bar.width
                    }

                    onMouseXChanged: {
                        var w = pressW + mouseX - pressX
                        if( w < 0 ) {
                            w = 0
                        }
                        if( w > width ) {
                            w = width
                        }
                        bar.width = w
                    }

                    onReleased: {

                    }
                }
            }

            signal drag(int time)
        }

    }
    */

    Connections {
        target: ImageModel
        onCurrentIndexChanged: {
            title.text = ImageModel.name
        }
    }

    // title
    Rectangle {
        id: title
        property alias text: titleText.text
        z: 2

        width: parent.width
        height: 60
        color: "#2f4f4f"
        Rectangle {
            id: btnReturn
            width: 60
            height: 60
            anchors.left: parent.left
            anchors.leftMargin: 5
            color: "transparent"
            Text {
                anchors.centerIn: parent
                font.family: "FontAwesome"
                font.pixelSize: parent.width * 0.65
                text: "\uf053"
                color: btnReturnArea.pressed ? "#6f9f9f" : "white"
            }
            MouseArea {
                id: btnReturnArea
                anchors.fill: parent
                onClicked: {
//                    imagePaintView.closeStream()
                    VideoPlayer.closeStream()
                    imagePlayer.state = "hide"
                }
            }

            Text {
                id: titleText
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.right
                font.pixelSize: 20
                color: "white"
//                text: qsTr("Photo")
            }
        }

        state: "show"
        states: [
            State {
                name: "hide"
                PropertyChanges {
                    target: title
                    y: 0 - height
                }
            },
            State {
                name: "show"
                PropertyChanges {
                    target: title
                    y: 0
                }
            }
        ]
        transitions: [
            Transition {
                from: "hide"
                to: "show"
                YAnimator {
                    target: title
                    duration: 160
                    easing.type: Easing.OutCurve
                }
            },
            Transition {
                from: "show"
                to: "hide"
                YAnimator {
                    target: title
                    duration: 160
                    easing.type: Easing.OutCurve
                }
            }
        ]
    }

    state: "hide"
    states: [
        State {
            name: "hide"
            PropertyChanges {
                target: imagePlayer
                opacity: 0
            }
        },
        State {
            name: "show"
            PropertyChanges {
                target: imagePlayer
                opacity: 1
            }
        }
    ]
    transitions: [
        Transition {
            from: "hide"
            to: "show"
            OpacityAnimator {
                target: imagePlayer
                duration: 200
            }
        },
        Transition {
            from: "show"
            to: "hide"
            OpacityAnimator {
                target: imagePlayer
                duration: 200
            }
        }
    ]

    onOpacityChanged: {
        if( opacity < 0.1 ) {
            if( visible === true ) {
                visible = false;
            }
        }
        if( opacity > 0.1 ) {
            if( visible === false ) {
                visible = true;
            }
        }
    }

//    FolderListModel {
//        id:scanModel
//        showDirs: false
//        nameFilters: ["*.png", "*.jpg", "*.jpeg", "*.gif","*.JPG","*.PNG", "*.bmp","*.BMP","*.GIF","*.gif"]
//        folder: Config.albumFolder
//    }

//    Component.onCompleted: {
//        console.log("floder = ", Config.albumFolder, width, height, photoScan.width)
//    }


    Rectangle {
        id: bottom

        // behavio 在屏幕旋转的时候 不会更新坐标
        // 需要替换为 states
//        Behavior on y {
//            YAnimator {
//                duration: 160
//                easing.type: Easing.OutCurve
//            }
//        }

        state: "show"
        states: [
            State {
                name: "hide"
                PropertyChanges {
                    target: bottom
                    y: imagePlayer.height
                }
            },
            State {
                name: "show"
                PropertyChanges {
                    target: bottom
                    y: imagePlayer.height - height
                }
            }
        ]
        transitions: [
            Transition {
                from: "hide"
                to: "show"
                YAnimator {
                    target: bottom
                    duration: 160
                    easing.type: Easing.OutCurve
                }
            },
            Transition {
                from: "show"
                to: "hide"
                YAnimator {
                    target: bottom
                    duration: 160
                    easing.type: Easing.OutCurve
                }
            }
        ]

//        x: 0
//        y: parent.height - height
        width: parent.width
        height: 60
        color: "#2f4f4f"
        z: 2
        GridView {
            id: miniPhtotList
            width: parent.width
            height: parent.height
            anchors.bottom: parent.bottom
            model: ImageModel
            cellWidth: width > height ? height : width
            cellHeight: cellWidth
            flow: GridView.FlowTopToBottom
            layoutDirection: GridView.LeftToRight
            leftMargin: width / 2 - cellWidth / 2
            rightMargin: width / 2 - cellWidth / 2

            // 底部滑动, 更改当前播放的图片
//            onContentXChanged: {
//                var index = Number((contentX + leftMargin) / cellWidth)
//                if( index > 0 && photoScan.currentIndex !== index ) {
//                    photoScan.positionViewAtIndex(index, ListView.Beginning)
//                }
//            }

            delegate: Rectangle {
                id: itembg
                width: miniPhtotList.cellWidth
                height: miniPhtotList.cellHeight
                color: "transparent"
                Image {
                    asynchronous: true
                    id: photo
                    cache: true
                    sourceSize: Qt.size(parent.width, parent.height)
//                    source: fileType == 0 ? path : ""
                    source: path
                    anchors.centerIn: parent
                    width: parent.width - 2
                    height: parent.height - 2
                    smooth: true
                    fillMode: Image.PreserveAspectCrop
                    Text {
                        visible: fileType === 1
                        anchors.centerIn: parent
                        font.family: "FontAwesome"
                        font.pixelSize: parent.width * 0.35
                        text: "\uf144"
                        color: mouseArea.pressed ? "#a0a0a0" : "white"
                    }
                }

                MouseArea{
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: {
                        miniPhtotList.positionViewAtIndex(index, ListView.Center)
                        photoScan.positionViewAtIndex(index, ListView.Beginning)
                    }
                }
            }
        }
    }
}
