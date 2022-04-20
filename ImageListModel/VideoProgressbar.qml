import QtQuick 2.0

Item {
    // 视频进度条工具栏
    // 分成两个区域
    // 按纽栏
    // 进度条

    Behavior on opacity {
        NumberAnimation {
            duration: 200
        }
    }

    MouseArea {
        anchors.fill: parent
    }

    Rectangle {
        id: progressBarItem
        anchors.fill: parent

        radius: 10
        color: "#D0505050"

        // 按钮栏
        Item {
            id: toolbar
            width: Config.isLandscape ? parent.width * 0.25 : parent.width
            height: Config.isLandscape ? parent.height : parent.height / 2
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            // 暂停按钮
            Text {
                id: btnPause
                text: VideoPlayer.playing ? "\uf04c" : "\uf04b"
                font.family: Config.fontLight
                font.pixelSize: 30
                anchors.centerIn: Config.isLandscape ?
                                      parent : undefined
                anchors.horizontalCenter: Config.isLandscape ?
                                              undefined : parent.horizontalCenter
                color: btnPauseArea.pressed ? "#a0a0a0" : "white"
                MouseArea {
                    id: btnPauseArea
                    anchors.fill: parent
                    onClicked: {
                        VideoPlayer.playPause()
                    }
                }
            }
        }

        // 进度条
        Item {
            id: progressbar
            width: Config.isLandscape ? parent.width * 0.75 : parent.width
            height: Config.isLandscape ? parent.height : parent.height / 2
            anchors.right: parent.right

            Loader {
                Component {
                    id: verticalProgressbar
                    // 进度条底条
                    Rectangle {
                        id: bottombar
                        width: progressbar.width * 0.65
                        height: 6
                        anchors.centerIn: parent
                        color: "#505050"
                        radius: 10

                        // 当前播放时间
                        Text {
                            anchors.right: parent.left
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: VideoPlayer.currentTime
                            font.pixelSize: 15
                            color: "white"
                        }

                        // 视频总时间
                        Text {
                            id: totalTime
                            anchors.left: parent.right
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: VideoPlayer.totalTime
                            font.pixelSize: 15
                            color: "white"
                        }

                        Connections {
                            target: VideoPlayer
                            onFrameUpdate: {
        //                        console.log("onframeupdate")
                                if( !mouseArea.pressed ) {
                                    rollbar.width = bottombar.width * VideoPlayer.progress
                                }
                            }
                        }

                        //  进度条
                        Rectangle {
                            id: rollbar
        //                    width: parent.width * VideoPlayer.progress
                            width: 0
                            height: parent.height
                            color: "white"
                            radius: 10
                        }

                        // 进度条球
                        Rectangle {
                            width: parent.height + 6
                            height: width
                            x: rollbar.width - width / 2
                            radius: width / 2
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            border.width: 1
                            border.color: "#a0a0a0"
                        }

                        MouseArea {
                            id: mouseArea
                            property real pressW
                            property real pressX
                            width: parent.width
                            height: parent.height * 6
        //                    enabled: false
                            anchors.verticalCenter: parent.verticalCenter
                            onPressed: {
                                pressX = mouseX
        //                        rollbar.width = pressX
                                pressW = rollbar.width
                            }

                            onMouseXChanged: {
                                var w = pressW + mouseX - pressX
                                if( w < 0 ) {
                                    w = 0
                                }
                                if( w > width ) {
                                    w = width
                                }
                                rollbar.width = w
                                VideoPlayer.seek(w / bottombar.width)
                            }

                            onReleased: {

                            }
                        }
                    }
                }

                Component {
                    id: horizontalProgressbar
                    // 进度条底条
                    Rectangle {
                        id: bottombar
                        width: progressbar.width * 0.85
                        height: 6
                        anchors.centerIn: parent
                        color: "#505050"
                        radius: 10

                        // 当前播放时间
                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.bottom
                            anchors.topMargin: 10
                            text: VideoPlayer.currentTime
                            font.pixelSize: 15
                            color: "white"
                        }

                        // 视频总时间
                        Text {
                            id: totalTime
                            anchors.right: parent.right
                            anchors.top: parent.bottom
                            anchors.topMargin: 10
                            text: VideoPlayer.totalTime
                            font.pixelSize: 15
                            color: "white"
                        }

                        Connections {
                            target: VideoPlayer
                            onFrameUpdate: {
                                if( !mouseArea.pressed ) {
                                    rollbar.width = bottombar.width * VideoPlayer.progress
                                }
                            }
                        }

                        //  进度条
                        Rectangle {
                            id: rollbar
                            width: 0
                            height: parent.height
                            color: "white"
                            radius: 10
                        }

                        // 进度条球
                        Rectangle {
                            width: parent.height + 6
                            height: width
                            x: rollbar.width - width / 2
                            radius: width / 2
                            color: "white"
                            anchors.verticalCenter: parent.verticalCenter
                            border.width: 1
                            border.color: "#a0a0a0"
                        }

                        MouseArea {
                            id: mouseArea
                            property real pressW
                            property real pressX
                            width: parent.width
                            height: parent.height * 6
                            anchors.verticalCenter: parent.verticalCenter
                            onPressed: {
                                pressX = mouseX
                                pressW = rollbar.width
                            }

                            onMouseXChanged: {
                                var w = pressW + mouseX - pressX
                                if( w < 0 ) {
                                    w = 0
                                }
                                if( w > width ) {
                                    w = width
                                }
                                rollbar.width = w
                                VideoPlayer.seek(w / bottombar.width)
                            }

                            onReleased: {

                            }
                        }
                    }
                }

                anchors.centerIn: parent
                sourceComponent: Config.isLandscape ?
                                     verticalProgressbar : horizontalProgressbar

                Component.onCompleted: console.log("is landscape", Config.isLandscape)
            }
        }
    }
}
