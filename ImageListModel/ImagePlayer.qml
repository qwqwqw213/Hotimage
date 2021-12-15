import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.4
import Qt.labs.folderlistmodel 2.14

import Custom.ImagePaintView 1.1

Rectangle{
    id: imagePlayer
//    property alias photoScanModel: photoScan.model
    anchors.fill: parent
    color: "black"

    function hideTitle() {
        if( title.state !== "hide" ) {
            title.state = "hide"
            bottom.y = height
        }
    }

    function autoTitle(state) {
        if( title.state === "show" ) {
            title.state = "hide"
            bottom.y = height
        }
        else {
            title.state = "show"
            bottom.y = height - bottom.height
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
//            opacity: imagePlayer.opacity
            width: photoScan.width
            height: photoScan.height
//            onItemClicked: {
//                if( title.state === "show" ) {
//                    title.state = "hide"
//                }
//                else {
//                    title.state = "show"
//                }
//            }
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
            imagePaintView.closeStream()
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

//    Rectangle {
//        id: zoomInRect
//        z: 2
//        x: 0
//        y: 0
//        width: parent.width
//        height: parent.height
//        color: "transparent"
//        Image {
//            id: zoomInImage
//            anchors.fill: parent
//            cache: true
//            asynchronous: true
//            fillMode: Image.PreserveAspectCrop
//        }

//        function show(x, y, w, h, path) {
//            console.log(x, y, w, h)
//            zoomInRect.x = x
//            zoomInRect.y = y
//            zoomInRect.width = w
//            zoomInRect.height = h
//            zoomInImage.source = path
//        }

//    }

    ImagePaintView {
        x: photoScan.currentIndex * photoScan.width - photoScan.contentX
        id: imagePaintView
        width: photoScan.width
        height: photoScan.height
        z: 1
        visible: playing
    }

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
                    imagePaintView.closeStream()
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
        Behavior on y {
            YAnimator {
                duration: 160
                easing.type: Easing.OutCurve
            }
        }

        x: 0
        y: parent.height - height
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
