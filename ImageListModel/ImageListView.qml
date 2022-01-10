import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: imageListView
//    anchors.fill: parent
//    width: parent.width
//    height: parent.height
//    y: height
    onVisibleChanged: {
    }

//    Behavior on y {
//        NumberAnimation {
//            duration: 300
//        }
//    }
    clip: true

    MouseArea {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        color: "black"

        GridView {
            id: imageList
            x: 0
            y: 60
            width: parent.width
            height: parent.height - 60
            cellWidth: imageListView.width > imageListView.height ?
                           (imageListView.width / 4) : (imageListView.width / 3)
            cellHeight: cellWidth
            model: ImageModel

//            bottomMargin: bottom.y < imageListView.height ? bottom.height : 0
            bottomMargin: imageListView.height - bottom.y
//            onBottomMarginChanged: {
//                console.log(bottomMargin, imageListView.height, bottom.y)
//            }

//            Component.onCompleted: positionViewAtEnd()

            delegate: Rectangle {
                id: delegate
                width: imageList.cellWidth
                height: imageList.cellHeight
                color: "transparent"

                Image {
                    asynchronous: true
//                    source: fileType == 0 ? path : ""
                    source: path
                    width: imageList.cellWidth - 2
                    height: imageList.cellHeight - 2
                    fillMode: Image.PreserveAspectCrop
                    anchors.centerIn: parent
                    smooth: true
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
                        visible: ImageModel.selectionStatus
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
                            imagePlayer.show(index,
                                             delegate.x + imageList.x + 1,
                                             delegate.y - imageList.contentY + imageList.y + 1,
                                             imageList.cellWidth - 2,
                                             imageList.cellHeight - 2,
                                             fileType === 0 ? path : "")
                        }
                    }
                    onPositionChanged: {
                        if( ImageModel.selectionStatus ) {
                        }
                    }
                }

            }

            ScrollBar.vertical: ScrollBar {

            }

            onContentYChanged: {
//                console.log(contentY, originY)
            }
        }

        // 选择区域
        MouseArea {
            anchors.fill: imageList
            visible: false
            onPressed: {
                console.log("pressed")
            }
        }

        // title
        Rectangle {
            width: parent.width
            height: 60
            color: "#2f4f4f"

            MouseArea {
                anchors.fill: parent
            }

            // 返回按钮
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
                        stackView.pop()
//                        imageListView.y = imageListView.height
                    }

                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.right
                    font.pixelSize: 20
                    color: "white"
                    text: qsTr("Photo")
                }
            }

            Rectangle {
                id: btnSelect
                width: 60
                height: 60
                anchors.right: parent.right
                anchors.rightMargin: 10
                color: "transparent"
                Text {
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: parent.width * 0.65
                    text: ImageModel.selectionStatus ? "\uf192" : "\uf10c"
                    color: btnSelectArea.pressed ? "#6f9f9f" : (ImageModel.selectionStatus ? "#6f9f9f" : "white")
                }
                MouseArea {
                    id: btnSelectArea
                    anchors.fill: parent
                    property bool selectionStatus: false
                    onClicked: {
                        selectionStatus = !selectionStatus
                        ImageModel.selectionStatus = selectionStatus

//                        bottom.y = ImageModel.selectionStatus ? imageListView.height - bottom.height : imageListView.height
                        bottom.state = ImageModel.selectionStatus ? "show" : "hide"
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.left
                    font.pixelSize: 20
                    color: "white"
                    text: qsTr("Choice")
                }
            }
        }

        // bottom
        Rectangle {
            id: bottom
            width: parent.width
            height: 60
            color: "#2f4f4f"
//            y: imageListView.height

            onYChanged: {
                var temp = imageList.contentHeight + (imageList.height * 2)

                var row = imageListView.width > imageListView.height ? 4 : 3
                var flag = (imageList.count % row == 0) ? 0 : 1
                var bottom = (Math.floor(imageList.count / row) + flag) * imageList.cellWidth - imageList.height
                bottom += imageList.originY
                if( imageList.contentY >= bottom && ready ) {
                    imageList.contentY = bottom + y
                }
            }

            // 删除按钮
            Rectangle {
                id: btnDelete
                width: 60
                height: 60
                anchors.right: parent.right
                anchors.rightMargin: 5
                color: "transparent"
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
                    font.pixelSize: 20
                    color: "white"
                    text: qsTr("Delete")
                }
            }

            state: "hide"
            states: [
                State {
                    name: "hide"
                    PropertyChanges {
                        target: bottom
                        y: imageListView.height
                    }
                },
                State {
                    name: "show"
                    PropertyChanges {
                        target: bottom
                        y: imageListView.height - bottom.height
                    }
                }
            ]
            transitions: [
                Transition {
                    from: "hide"
                    to: "show"
                    PropertyAnimation {
                        target: bottom
                        property: "y"
                        duration: 160
                        easing.type: Easing.OutCurve
                    }
                },
                Transition {
                    from: "show"
                    to: "hide"
                    PropertyAnimation {
                        target: bottom
                        property: "y"
                        duration: 160
                        easing.type: Easing.OutCurve
                    }
                }
            ]

            // Behavior的效果不好
            // 在 StackView 切换界面时, 会先从顶部移动到底部
            /*
            Behavior on y {
                NumberAnimation {
                    duration: 200
                }
            }
            */
        }

    }

    Connections {
        target: ImageModel
        onCurrentIndexChanged: {

        }
    }

    ImagePlayer {
        id: imagePlayer
        visible: false

        onCurrentIndexChanged: {
//            imageList.positionViewAtIndex(index, ListView.End)

            var row = imageListView.width > imageListView.height ? 4 : 3
            var flag = (imageList.count % row == 0) ? 0 : 1
            var bottom = (Math.floor(imageList.count / row) + flag) * imageList.cellWidth - imageList.height
            var y = Math.floor(index / row) * imageList.cellHeight;
            if( y > bottom ) {
                y = bottom
            }
            imageList.contentY = y
        }

//        visible: true
    }

//    StackView.onActivating: {
//        imageList.positionViewAtEnd()
//    }

    Component.onCompleted: {
//        imageList.positionViewAtEnd()
//        console.log(imageList.count)
        imageList.positionViewAtIndex(imageList.count - 1, ListView.End)
    }

    property bool ready: false
    StackView.onActivated: {
        PhoneApi.setRotationScreen(-1)
        ready = true
    }
    StackView.onDeactivated: {
        ready = false
    }

    signal currentIndexChanged(var index, var source)
}
