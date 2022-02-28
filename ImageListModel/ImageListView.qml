import QtQuick 2.14
import QtQuick.Controls 2.14

Item {
    id: imageListView
    clip: true

    property alias interactive: imageList.interactive

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

    Rectangle {
        anchors.fill: parent
        color: "black"

        GridView {
            id: imageList
            property int row: imageListView.width > imageListView.height ?
                                  5 : 3
            property real maxContentY: contentHeight - height + originY

            anchors.fill: parent
            cellWidth: imageListView.width > imageListView.height ?
                           (imageListView.width / row) : (imageListView.width / row)
            cellHeight: cellWidth
            model: ImageModel

            topMargin: title.height
            bottomMargin: bottom.height + bottom.posY
            onBottomMarginChanged: {
                if( maxContentY > 0 && contentY >= (maxContentY - 5) ) {
                    contentY = maxContentY + bottomMargin
                }
            }

            delegate: Rectangle {
                id: delegate
                width: imageList.cellWidth
                height: imageList.cellHeight
                color: "transparent"
                clip: true
                visible: ImageModel.currentIndex === index ?
                             (imagePlayer.isShow ? false : true) : true

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

                    Text {
                        visible: fileType === 1
                        anchors.centerIn: parent
                        font.family: "FontAwesome"
                        font.pixelSize: parent.width * 0.35
                        text: "\uf144"
                        color: ImageModel.selectionStatus ? "white" : (mouseArea.pressed ? "#a0a0a0" : "white")
                    }

                    // 选择标识
                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        font.family: "FontAwesome"
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
                            ImageModel.currentIndex = index
                            var x = delegate.x
                            var y = (delegate.y - imageList.originY)
                                    - (imageList.contentY - imageList.originY)
                                    + imageList.y
                            var w = delegate.width
                            var h = delegate.height
                            imagePlayer.zoom(index, path, x, y, w, h,
                                             delegateSource.paintedWidth, delegateSource.paintedHeight)
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
        Rectangle {
            id: title

            property real leftRightMargin: 30

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
                        stackView.pop()
                    }
                }

                Text {
                    anchors.left: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: 15
                    text: qsTr("Photo")
                    color: "white"
                }
            }

            // 选择按钮
            AbstractButton {
                id: btnChoice
                width: btnChoiceText.contentWidth + 20
                height: btnChoiceText.contentHeight + 20
                y: (parent.height - Config.topMargin - height) / 2 + Config.topMargin
                anchors.right: parent.right
                anchors.rightMargin: parent.leftRightMargin

                background: Rectangle {
                    radius: height / 2
                    color: btnChoice.pressed ? "#1f3f3f" : "#3f5f5f"
                }
                Text {
                    id: btnChoiceText
                    anchors.centerIn: parent
                    font.pointSize: 15
                    color: "white"
                    text: ImageModel.selectionStatus ?
                              qsTr("Cancel") : qsTr("Choice")
                }
                onClicked: {
                    ImageModel.selectionStatus = !ImageModel.selectionStatus
                    bottom.posY = ImageModel.selectionStatus ? 0 : 0 - bottom.height
                }
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
                    font.family: "FontAwesome"
                    font.pixelSize: parent.width * 0.65
                    text: "\uf1f8"
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
                    font.pointSize: 15
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
