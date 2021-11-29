import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    visible: true
    id: imageListView
    width: parent.width
    height: parent.height
    GridView {
        id: imageList
        anchors.fill: parent
        cellWidth: width / 4
        cellHeight: width / 4
        model: imageModel

        delegate: Rectangle {
            width: imageList.cellWidth
            height: imageList.cellHeight
            color: "transparent"
            Image {
                id: photo
                asynchronous: true
                source: path
                width: parent.width - 2
                height: parent.height - 2
                fillMode: Image.PreserveAspectCrop
                anchors.centerIn: parent
                smooth: true
                onStatusChanged: {
                    if( status === Image.Ready ) {
                        photoLoading.running = false
                    }
                    else if( status === Image.Error ) {
                        console.log("load image error:", source)
                        photoLoading.running = false
                    }
                    else {
                        photoLoading.running = true
                    }
                }
            }

            MouseArea {
                id:mouseArea
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    imageModel.currentIndex = index
                    currentIndexChanged(index, photo.source)
                }
            }

            BusyIndicator {
                id: photoLoading
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }

//    XAnimator on x{
//        from: imageListView.x
//        to: 0
//        duration: 300
//        loops: 1
//        easing.type: Easing.OutCubic
//    }

//    OpacityAnimator on opacity {
//        from: 0
//        to: 1
//        duration: 300
//        loops: 1
//        easing.type: Easing.OutCubic
//    }

    Connections {
        target: imageModel
        onCurrentIndexChanged: {

        }
    }

    signal currentIndexChanged(var index, var source)
}
