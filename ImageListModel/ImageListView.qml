import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: imageListView
//    anchors.fill: parent
//    width: parent.width
//    height: parent.height
//    y: height
    onVisibleChanged: {
        if( !visible ) {
            ImageModel.selectionStatus = false
        }
    }

//    Behavior on y {
//        NumberAnimation {
//            duration: 300
//        }
//    }

    MouseArea {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        color: "black"

        GridView {
            id: imageList
            y: 60
            width: parent.width
            height: parent.height - 60
            cellWidth: width > height ? (width / 4) : (width / 3)
            cellHeight: cellWidth
            model: ImageModel

//            Component.onCompleted: positionViewAtEnd()

            delegate: Rectangle {
                id: delegate
                width: imageList.cellWidth
                height: imageList.cellHeight
                color: "transparent"

                Image {
                    asynchronous: true
                    source: fileType == 0 ? path : ""
                    width: imageList.cellWidth - 2
                    height: imageList.cellHeight - 2
                    fillMode: Image.PreserveAspectCrop
                    anchors.centerIn: parent
                    smooth: true

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

                    MouseArea {
                        id:mouseArea
                        anchors.fill: parent
                        onClicked: {

                            if( ImageModel.selectionStatus === 0 ) {
                                ImageModel.currentIndex = index
                                imagePlayer.show(index)
                            }

    //                        var component = Qt.createComponent("qrc:/ImageListModel/ImagePlayer.qml")
    //                        if( component.status === Component.Ready ) {
    //                            var play = component.createObject(imageListView)
    //                            play.show(index)
    //                        }


                            if( ImageModel.selectionStatus ) {
                                selection = !selection
                            }
                        }
                        onPositionChanged: {
                            if( ImageModel.selectionStatus ) {
                            }
                        }
                    }
                }


//                Loader {
//                    anchors.centerIn: parent
//                    Component {
//                        id: photoType
//                        Image {
//                            asynchronous: true
//                            source: path
//                            width: imageList.cellWidth - 2
//                            height: imageList.cellHeight - 2
//                            fillMode: Image.PreserveAspectCrop
//                            anchors.centerIn: parent
//                            smooth: true
////                            onStatusChanged: {
////                                if( status === Image.Ready ) {
////                                    photoLoading.running = false
////                                }
////                                else if( status === Image.Error ) {
////                                    photoLoading.running = false
////                                }
////                                else {
////                                    photoLoading.running = true
////                                }
////                            }

//                            // 选择标识
//                            Text {
//                                anchors.right: parent.right
//                                anchors.rightMargin: 5
//                                anchors.top: parent.top
//                                anchors.topMargin: 5
//                                font.family: "FontAwesome"
//                                font.pixelSize: 30
//                                text: selection ? "\uf192" : "\uf10c"
//                                color: selection ? "#6f9f9f" : "white"
//                                visible: ImageModel.selectionStatus
//                            }

////                            BusyIndicator {
////                                id: photoLoading
////                                anchors.centerIn: parent
////                            }
//                        }
//                    }

//                    Component {
//                        id: videoType
//                        Rectangle {
//                            width: imageList.cellWidth - 2
//                            height: imageList.cellHeight - 2
//                            color: "red"
//                        }
//                    }

//                    sourceComponent: fileType == 0 ? photoType : videoType
//                    onStatusChanged: {
//                        if( status == Loader.Loading ) {
//                            photoLoading.running = true
//                        }
//                        else if( status == Loader.Ready ) {
//                            photoLoading.running = false
//                        }
//                    }

//                    BusyIndicator {
//                        id: photoLoading
//                        anchors.centerIn: parent
//                    }
//                }

                // 选择 鼠标事件
//                MouseArea {
//                    id:mouseArea
//                    anchors.fill: parent
//                    onClicked: {

//                        if( ImageModel.selectionStatus === 0 ) {
//                            ImageModel.currentIndex = index
//                            imagePlayer.show(index)
//                        }

////                        var component = Qt.createComponent("qrc:/ImageListModel/ImagePlayer.qml")
////                        if( component.status === Component.Ready ) {
////                            var play = component.createObject(imageListView)
////                            play.show(index)
////                        }


//                        if( ImageModel.selectionStatus ) {
//                            selection = !selection
//                        }
//                    }
//                    onPositionChanged: {
//                        if( ImageModel.selectionStatus ) {
//                        }
//                    }
//                }

            }

            ScrollBar.vertical: ScrollBar {

            }

            onContentYChanged: {
                var temp = contentHeight - height - 60;
//                if( contentY > temp ) {
//                    temp = 60 - (contentY - temp)
//                    y = temp > 0 ? temp : 0
//                }
//                if( contentY < 60 ) {
//                    temp = 60 - contentY
//                    y = temp < 60 ? temp : 60
//                }
//                if( contentY <= 60 ) {
//                    if( y < 60 ) {
//                        y = 60
//                    }
//                }
//                if( contentY > temp ) {
//                    if( y > 0 ) {
//                        y = 0
//                    }
//                }
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
                        AndroidApi.setRotationScreen(0)
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
                    text: btnSelectArea.selectionStatus ? "\uf192" : "\uf10c"
                    color: btnSelectArea.pressed ? "#6f9f9f" : (btnSelectArea.selectionStatus ? "#6f9f9f" : "white")
                }
                MouseArea {
                    id: btnSelectArea
                    anchors.fill: parent
                    property bool selectionStatus: false
                    onClicked: {
                        selectionStatus = !selectionStatus
                        ImageModel.selectionStatus = selectionStatus

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
//            y: parent.height - 60

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
                        y: imageListView.height - height
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
//        visible: true
    }

//    StackView.onActivating: {
//        imageList.positionViewAtEnd()
//    }

    Component.onCompleted: {
        imageList.positionViewAtEnd()
    }

    StackView.onActivated: {
        AndroidApi.setRotationScreen(-1)
    }

    signal currentIndexChanged(var index, var source)
}
