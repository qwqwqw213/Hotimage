import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.4
import Qt.labs.folderlistmodel 2.14


Rectangle{
    id: imagePlayer
//    property alias photoScanModel: photoScan.model
    anchors.fill: parent
    color: "black"

    function show(index) {
        photoScan.contentX = index * width
        visible = true
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
        delegate: AlbumScanDelegate {
            opacity: imagePlayer.opacity
            width: photoScan.width
            height: photoScan.height
            onItemClicked: {
                if( title.state === "show" ) {
                    title.state = "hide"
                }
                else {
                    title.state = "show"
                }
            }
            Component.onCompleted: {
                if( index == photoScan.currentIndex ) {
                    title.text = name
                }
            }
        }
        currentIndex: 0

        property real maxContentX: width * (ImageModel.rowCount() - 1)
        onMovementStarted: {

        }

        onMovementEnded: {

        }

        onCurrentIndexChanged: {
            ImageModel.currentIndex = currentIndex
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
            anchors.leftMargin: 30
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

//    GridView {
//        id: photoList
//        width: parent.width
//        height: parent.height * 0.1
////        anchors.left: parent.left
//        anchors.bottom: parent.bottom
//        model: scanModel
//        cellWidth: width > height ? height : width
//        cellHeight: cellWidth
//        flow: GridView.FlowTopToBottom
//        layoutDirection: GridView.LeftToRight
//        highlightFollowsCurrentItem: true
//        z: 2

////        x: width / 2 - cellWidth / 2

//        property int maxContentX: cellWidth * scanModel.count - ((width / cellWidth) * cellWidth)
//        onContentXChanged: {
//            if( (atXBeginning == false)
//                    && (atXEnd == false) ) {
////                photoScan.positionViewAtIndex(contentX / cellWidth, ListView.Beginning)
//            }
//        }

//        delegate: Rectangle {
//            id: itembg
//            width: photoList.cellWidth
//            height: photoList.cellHeight
//            color: "transparent"

//            Image {
//                asynchronous: true
//                id: photo
//                cache: true
//                sourceSize: Qt.size(parent.width, parent.height)
//                source: scanModel.folder + fileName
//                anchors.centerIn: parent
//                width: parent.width - 2
//                height: parent.height - 2
//                smooth: true
//                fillMode: Image.PreserveAspectCrop
//            }

//            MouseArea{
//                anchors.fill: parent
//                onClicked: {
//                    photoList.currentIndex = index
//                    photoList.positionViewAtIndex(index, ListView.Center)
//                    console.log("gridView item clicked", photoList.currentIndex)
//                    photoScan.positionViewAtIndex(index, ListView.Beginning)
//                }
//            }
//        }

//        Component.onCompleted: console.log(x, y, width, height, cellWidth, cellHeight)
//    }
}
