import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    visible: true
    id: imageListView
    width: parent.width
    height: parent.height

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

            ScrollBar.vertical: ScrollBar {

            }
        }

        // title
        Rectangle {
            width: parent.width
            height: 60
            color: "#2f4f4f"

            Text {
                id: btnReturn
                anchors.left: parent.left
                anchors.leftMargin: 30
                anchors.verticalCenter: parent.verticalCenter
                text: "\uf053"
                font.family: "FontAwesome"
                font.pixelSize: 36
                color: btnReturnArea.pressed ? "#A0A0A0" : "white"
            }

            MouseArea {
                id: btnReturnArea
                anchors.fill: btnReturn
                onPressed: {
                    console.log("back")
                }
            }

            Text {
                anchors.left: btnReturn.right
                anchors.leftMargin: 18
                anchors.verticalCenter: btnReturn.verticalCenter
                font.pixelSize: 30
                text: qsTr("Photo")
                color: "white"
            }
        }

    }

    Connections {
        target: imageModel
        onCurrentIndexChanged: {

        }
    }

    signal currentIndexChanged(var index, var source)
}
