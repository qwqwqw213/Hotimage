import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.4
import Qt.labs.folderlistmodel 2.14


Rectangle{
    id:root
//    property alias photoScanModel: photoScan.model
    width: parent.width
    height: parent.height

    ListView {
        id: photoScan
        anchors.fill: parent
        model: scanModel
        cacheBuffer: 5
        clip: true
        z:1
//        maximumFlickVelocity:7000  //设置滑动的最大速度
        highlightRangeMode: ListView.NoHighlightRange
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        delegate:AlbumScanDelegate{
            width: photoScan.width
            height: photoScan.height
        }

        property real maxContentX: width * (scanModel.count - 1)
        onMovementStarted: {
            console.log("start", contentX)
        }

        onContentXChanged: {
            if( atXBeginning < 0 ) {
                photoList.contentX = contentX
            }
            else if( atXEnd ) {
                photoList.contentX = photoList.maxContentX + contentX - maxContentX
            }
            else {

            }
        }

        onMovementEnded: {
            photoList.positionViewAtIndex(contentX / width, ListView.Center)
        }
    }

    FolderListModel {
        id:scanModel
        showDirs: false
        nameFilters: ["*.png", "*.jpg", "*.jpeg", "*.gif","*.JPG","*.PNG", "*.bmp","*.BMP","*.GIF","*.gif"]
        folder: Config.albumFolder
    }

    Component.onCompleted: {
        console.log("floder = ", Config.albumFolder, width, height, photoScan.width)
    }

    GridView {
        id: photoList
        width: parent.width
        height: parent.height * 0.1
//        anchors.left: parent.left
        anchors.bottom: parent.bottom
        model: scanModel
        cellWidth: width > height ? height : width
        cellHeight: cellWidth
        flow: GridView.FlowTopToBottom
        layoutDirection: GridView.LeftToRight
        highlightFollowsCurrentItem: true
        z: 2

//        x: width / 2 - cellWidth / 2

        property int maxContentX: cellWidth * scanModel.count - ((width / cellWidth) * cellWidth)
        onContentXChanged: {
            if( (atXBeginning == false)
                    && (atXEnd == false) ) {
//                photoScan.positionViewAtIndex(contentX / cellWidth, ListView.Beginning)
            }
        }

        delegate: Rectangle {
            id: itembg
            width: photoList.cellWidth
            height: photoList.cellHeight
            color: "transparent"

            Image {
                asynchronous: true
                id: photo
                cache: true
                sourceSize: Qt.size(parent.width, parent.height)
                source: scanModel.folder + fileName
                anchors.centerIn: parent
                width: parent.width - 2
                height: parent.height - 2
                smooth: true
                fillMode: Image.PreserveAspectCrop
            }

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    photoList.currentIndex = index
                    photoList.positionViewAtIndex(index, ListView.Center)
                    console.log("gridView item clicked", photoList.currentIndex)
                    photoScan.positionViewAtIndex(index, ListView.Beginning)
                }
            }
        }

        Component.onCompleted: console.log(x, y, width, height, cellWidth, cellHeight)
    }
}
