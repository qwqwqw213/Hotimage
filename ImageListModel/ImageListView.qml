import QtQuick 2.14
import QtQuick.Controls 2.14

import "../BackTitle"

Item {
    clip: true

//    property alias interactive: imageList.interactive

    Rectangle {
        anchors.fill: parent
        color: "black"

        GridView {
            id: imageList
            function itemRect(index) {
                var item = imageList.itemAtIndex(index)
                if( item === null ) {
                    imageList.positionViewAtIndex(index, ListView.Center)
                    return itemRect(index)
                }

                var x = item.x
                var y = Math.round((item.y - imageList.originY)
                                   - (imageList.contentY - imageList.originY)
                                   + imageList.y)
                var w = item.width
                var h = item.height

                /*
                 *  当从ImagePlayer返回的时候
                 *  当前图片在GridView中的坐标超出了屏幕
                 *  重新定位GridView的Content Y
                 */
                if( y < imageList.y || (y + h) > imageList.height ) {
                    console.log("reset content y", index, y, y + h, imageList.y, imageList.height)
                    imageList.positionViewAtIndex(index, ListView.Center)
                    return itemRect(index)
                }

                return {
                    x: x,
                    y: y,
                    w: w,
                    h: h,
                    pw: item.pw,
                    ph: item.ph
                }
            }

            property int row: parent.width > parent.height ?
                                  5 : 3
            property real maxContentY: contentHeight - height + originY

            anchors.fill: parent
            cellWidth: parent.width > parent.height ?
                           (parent.width / row) : (parent.width / row)
            cellHeight: cellWidth
            model: ImageModel

            topMargin: title.height
            bottomMargin: bottom.height + bottom.posY
            onBottomMarginChanged: {
                if( maxContentY > 0 && contentY >= (maxContentY - 5) ) {
                    contentY = maxContentY + bottomMargin
                }
            }

            delegate: Item {
                id: delegate
                width: imageList.cellWidth
                height: imageList.cellHeight
                clip: true
                visible: ImageModel.currentIndex === index ?
                             (imagePlayer.visible ? false : true) : true

                property alias pw: delegateSource.paintedWidth
                property alias ph: delegateSource.paintedHeight

                Image {
                    id: delegateSource
                    asynchronous: true
                    source: path
                    width: imageList.cellWidth - 2
                    height: imageList.cellHeight - 2
                    anchors.centerIn: parent

                    /*
                     *  指定源大小可以减少缓存
                     */
                    sourceSize: Qt.size(width, height)
                    smooth: false
                    fillMode: Image.PreserveAspectCrop
                    cache: false

//                    Text {
//                        visible: fileType === 1
//                        anchors.centerIn: parent
//                        font.family: Config.fontLight
//                        font.pixelSize: parent.width * 0.35
//                        text: "\uf144"
//                        color: "white"
//                    }
                    Text {
                        color: "white"
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 10
                        font.pixelSize: 15
                        text: videoTotalTime
                    }

                    // 选择标识
                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        font.family: Config.fontLight
                        font.pixelSize: 30
                        text: selection ? "\uf192" : "\uf10c"
                        color: selection ? "#6f9f9f" : "white"
//                        visible: ImageModel.selectionStatus
                        opacity: ImageModel.selectionStatus ? 1 : 0
                        Behavior on opacity {
                            OpacityAnimator {
                                duration: 150
                            }
                        }
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: {
                        if( ImageModel.selectionStatus ) {
                            selection = !selection
                        }
                        else {
                            imagePlayer.open(index)
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                contentItem: Rectangle {
                    implicitWidth: 5
                    color: "#b0505050"
                    radius: implicitWidth / 2
                }
            }
        }

        // title
        BackTitle {
            id: title
            width: parent.width
            height: 60 + Config.topMargin
            anchors.left: parent.left
            anchors.top: parent.top
            titleText: qsTr("Picture")

            buttonText: ImageModel.selectionStatus ?
                            qsTr("Cancel") : qsTr("Choice")
            onButtonClicked: {
                ImageModel.selectionStatus = !ImageModel.selectionStatus
                bottom.posY = ImageModel.selectionStatus ? 0 : 0 - bottom.height
            }
        }

        // bottom
        Rectangle {
            id: bottom
            width: parent.width
            height: 60 + Config.bottomMargin
            color: "#2f4f4f"

            property real posY: 0 - height
            Behavior on posY {
                NumberAnimation { duration: 160 }
            }

            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.bottomMargin: posY

            // 删除按钮
            Item {
                id: btnDelete
                width: parent.height - Config.bottomMargin
                height: width
                anchors.right: parent.right
                anchors.rightMargin: 5
                Text {
                    anchors.centerIn: parent
                    font.family: Config.fontRegular
                    font.pixelSize: parent.width * 0.65
                    text: "\uf2ed"
                    color: btnDeleteArea.pressed ? "#6f9f9f" : "white"
                }
                MouseArea {
                    id: btnDeleteArea
                    anchors.fill: parent
                    onClicked: ImageModel.removeSelection()
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.left
                    color: "white"
                    text: qsTr("Delete")
                }
            }
        }

    }

    ImagePlayer {
        id: imagePlayer
        anchors.fill: parent
        visible: false
    }

    property bool ready: false
    StackView.onActivated: {
        ready = true
    }

    property bool init: false
    StackView.onActivating: {
        if( !init ) {
            init = true
            var y = imageList.contentHeight - imageList.height + imageList.originY
            if( y < 0 ) {
                y = 0
            }
            imageList.contentY = y
        }
        else {
            imageList.contentY = oldContentY
        }
    }

    property real oldContentY: 0
    StackView.onDeactivated: {
        ready = false
        oldContentY = imageList.contentY
    }
}
