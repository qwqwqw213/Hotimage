import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.Window 2.12
import QtQuick.Shapes 1.14
import Qt.labs.platform 1.1

import Custom.ImageView 1.1

import "./ImageListModel"
import "./Setting"
import "./Loading"
import "./MessageBox"

Window {
    id: window
    visible: true
    width: Config.width
    height: Config.height
    title: qsTr("Hotimage")

    onVisibleChanged: {
        console.log("main visble status:", visible)
    }

    property real oldRotation: 0
    PropertyAnimation {
        id: rotationAnimation
        property: "oldRotation"
        target: window
        from: window.oldRotation
        to: Config.rotation
        duration: 100
        easing.type: Easing.OutCurve
    }

    Connections {
        target: Config
        onRotationChanged: {
            rotationAnimation.running = true
        }
    }

    Rectangle {
        id: mainView
//        anchors.fill: parent
        color: "black"

        property bool videoPlay: true
        StackView.onDeactivated: {
            videoPlay = false
        }
        StackView.onActivating: {
            videoPlay = true
        }

        // 顶部按钮栏
        Rectangle {
            width: parent.width * 0.1
            height: parent.height
            color: "black"

            // 设置按钮
            Rectangle {
                id: btnSetting
                width: parent.width * 0.75
                height: width
                anchors.top: parent.top
                anchors.topMargin: height / 2
                anchors.left: btnShutter.left
                color: "transparent"
//                color: "red"

                Text {
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: btnSetting.width * 0.8
                    text: "\uf013"
                    color: btnSettingArea.pressed ? "#a0a0a0" : "white"
                    rotation: window.oldRotation
                }

                MouseArea {
                    id: btnSettingArea
                    anchors.fill: parent
                    onReleased: {
                        setting.open()
                    }
                }
            }

            // 快门刷新按钮
            Rectangle {
                id: btnShutter
                width: parent.width * 0.75
                height: width
                anchors.centerIn: parent
                color: "transparent"

                Text {
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: btnSetting.width * 0.8
                    text: "\uf021"
                    color: btnShutterArea.pressed ? "#a0a0a0" : "white"
                    rotation: window.oldRotation
                }

                MouseArea {
                    id: btnShutterArea
                    anchors.fill: parent
                    onReleased: {
                        TcpCamera.shutter()
                    }
                }
            }

            // 温度按钮
            Rectangle {
                id: btnTempSetting
                width: parent.width * 0.75
                height: width
                anchors.bottom: parent.bottom
                anchors.bottomMargin: height / 2
                anchors.left: btnShutter.left
                color: "transparent"

                Text {
                    id: icon
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: btnSetting.width * 0.8
                    text: "\uf2cb"
                    color: TcpCamera.showTemp ? "#dc143c" : "#696969"
                    rotation: window.oldRotation
                }

                MouseArea {
                    id: btnTempSettingArea
                    anchors.fill: parent
                    onReleased: {
                        TcpCamera.showTemp = !TcpCamera.showTemp
                    }
                }
            }
        }

//        ImageView {
//            id: cameraFrame
//            width: parent.width * 0.7
//            height: parent.height
//            x: parent.width * 0.1
//            y: 0
//            camera: TcpCamera

//            // 录像标志
//            Text {
//                anchors.left: parent.left
//                anchors.leftMargin: 10
//                anchors.top: parent.top
//                anchors.topMargin: 10
//                font.family: "FontAwesome"
//                font.pixelSize: 30
//                text: "\uf03d" + " " + TcpCamera.recordTime
//                color: "red"
//                visible: TcpCamera.encoding

//                SequentialAnimation on color {
//                    loops: Animation.Infinite
//                    running: visible
//                    ColorAnimation {
//                        from: "red"
//                        to: "black"
//                        duration: 800
//                    }
//                    ColorAnimation {
//                        from: "black"
//                        to: "red"
//                        duration: 800
//                    }
//                }
//            }
//        }

        // 摄像头图像
        Image {
            id: cameraFrame
            width: parent.width * 0.7
            height: parent.height
            x: parent.width * 0.1
            y: 0
            source: mainView.videoPlay ?
                        (TcpCamera.isConnected ? TcpCamera.videoFrameUrl : "") : ""

            // 录像标志
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: 10
                font.family: "FontAwesome"
                font.pixelSize: 30
                text: "\uf03d" + " " + TcpCamera.recordTime
                color: "red"
                visible: TcpCamera.encoding

                SequentialAnimation on color {
                    loops: Animation.Infinite
                    running: visible
                    ColorAnimation {
                        from: "red"
                        to: "black"
                        duration: 800
                    }
                    ColorAnimation {
                        from: "black"
                        to: "red"
                        duration: 800
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
//                    Config.setRotation()
                    messagebox.showMsg("test message boxtest message boxtest message boxtest message boxtest message boxtest message boxtest message boxtest ")
                }
            }
        }

        // 截图动画
        Rectangle {
            id: captureAnimationView
            visible: false
            color: "black"
            anchors.fill: parent

            NumberAnimation {
                id: captureAnimation
                running: false
                target: captureAnimationView
                property: "opacity"
                duration: 500
                from: 1
                to: 0
                easing.type: Easing.OutQuad


                onFinished: {
                    captureAnimationView.visible = false
                }
            }

            function start() {
                captureAnimationView.visible = true
                captureAnimationView.opacity = 1.0
                captureAnimation.running = true;
            }
        }

        // 底部按钮栏
        Rectangle {
            id: bottomTool
            width: parent.width * 0.2
            height: parent.height
            color: "black"
            anchors.right: parent.right

            // 截图按钮
            Rectangle {
                id: btnCapture
                width: parent.width * 0.5
                height: width
                anchors.centerIn: parent
                color: "transparent"
                border.color: btnCaptureArea.pressed ? "#A0A0A0" : "white"
                border.width: 5
                radius: 100

                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.75 * btnCapture.m_scale
                    height: width
                    color: btnCaptureArea.pressed ? "#F01010" : "white"
                    radius: 100
                }

                MouseArea {
                    id: btnCaptureArea
                    anchors.fill: parent
                    pressAndHoldInterval: 100
                    onReleased: {
                        btnCaptureTimer.start()
                        TcpCamera.capture()
                        captureAnimationView.start()
                    }
                    onPressAndHold: {
                        btnCaptureTimer.start()
                    }
                }

                property var m_scale: 1.0
                Timer {
                    id: btnCaptureTimer
                    interval: 10
                    running: false
                    repeat: true
                    onTriggered: {
                        if( btnCaptureArea.pressed ) {
                            btnCapture.m_scale -= 0.1;
                            if( btnCapture.m_scale < 0.6 ) {
                                btnCapture.m_scale = 0.6;
                                btnCaptureTimer.stop()
                            }
                        }
                        else {
                            btnCapture.m_scale += 0.1;
                            if( btnCapture.m_scale > 0.9 ) {
                                btnCapture.m_scale = 1.0;
                                btnCaptureTimer.stop()
                            }
                        }
                    }
                }
            }

            // 相册按钮
            Rectangle {
                id: btnPhoto
                width: parent.width * 0.5
                height: width
                anchors.left: btnCapture.left
                anchors.top: parent.top
                anchors.topMargin: height / 2
                color: "transparent"

                Rectangle {
                    width: btnPhoto.width * 0.8 * btnPhoto.m_scale
                    height: btnPhoto.height * 0.8 * btnPhoto.m_scale
                    rotation: window.oldRotation
                    color: "transparent"
                    border.color: "white"
                    border.width: 1
                    anchors.centerIn: parent
                }

                Loader {
                    Component {
                        id: imageType
                        Image {
                            source: ImageModel.lastImagePath
                            width: btnPhoto.width * 0.8 * btnPhoto.m_scale
                            height: btnPhoto.height * 0.8 * btnPhoto.m_scale
                            rotation: window.oldRotation
                        }
                    }
                    Component {
                        id: videoType
                        Rectangle {
                            width: btnPhoto.width * 0.8 * btnPhoto.m_scale
                            height: btnPhoto.height * 0.8 * btnPhoto.m_scale
                            rotation: window.oldRotation
                            color: "transparent"
                            Text {
                                anchors.centerIn: parent
                                font.family: "FontAwesome"
                                font.pixelSize: parent.width * 0.5
                                text: "\uf144"
                                color: "white"
                                rotation: oldRotation
                            }
                        }
                    }
                    anchors.centerIn: parent
                    sourceComponent: ImageModel.lastType === 0 ? imageType : videoType
                }

                MouseArea {
                    id: btnPhotoArea
                    anchors.fill: parent
                    onPressed: {
                        btnPhoto.m_scale = 0.9
                    }
                    onReleased: {
                        btnPhoto.m_scale = 1.0
//                        stackView.push("qrc:/ImageListModel/ImageListView.qml")
                        stackView.push(imageListView)
//                        imageListView.y = 0
                    }
                }

                Connections {
                    target: ImageModel
                    onAddNewFile: {
                        btnPhoto.m_scale = 0.5
                        btnPhotoTimer.start()
                    }
                }

                property var m_scale: 1.0
                Timer {
                    id: btnPhotoTimer
                    interval: 10
                    running: false
                    repeat: true
                    onTriggered: {
                        btnPhoto.m_scale += 0.1
                        if( btnPhoto.m_scale > 0.9 ) {
                            btnPhoto.m_scale = 1.0
                            btnPhotoTimer.stop()
                        }
                    }
                }
            }

            // 录像按钮
            Rectangle {
                id: btnRecord
                width: parent.width * 0.5
                height: width
                anchors.left: btnCapture.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: height / 2
                color: "transparent"
                Text {
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: btnRecord.width * 0.75
                    text: "\uf03d"
                    color: btnRecordArea.pressed ? "#a0a0a0" : "white"
                    rotation: oldRotation
                }

                MouseArea {
                    id: btnRecordArea
                    anchors.fill: parent
                    onReleased: {
                        TcpCamera.openRecode()
                    }
                }
            }
        }

        MouseArea {
            id: mainViewArea
            anchors.fill: parent
            enabled: false
            onReleased: {
                if( setting.visible ) {
                    setting.tohide = true
                }
                enabled = false
            }
        }

//        ImagePlayer {}

        Setting {
            id: setting
            width: parent.width * 0.5
            height: parent.height
        }
    }

    ImageListView {
        id: imageListView
//        visible: false
//        y: parent.height
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainView

        pushEnter: Transition {
            XAnimator {
                from: width
                to: 0
                duration: 200
                easing.type: Easing.OutCurve
            }
        }
        pushExit: Transition {
            XAnimator {
                from: 0
                to: 0 - width
                duration: 200
                easing.type: Easing.OutCurve
            }
        }
        popEnter: Transition {
            XAnimator {
                from: 0 - width
                to: 0
                duration: 200
                easing.type: Easing.OutCurve
            }
        }
        popExit: Transition {
            XAnimator {
                from: 0
                to: width
                duration: 200
                easing.type: Easing.OutCurve
            }
        }


    }

    Loading {
        visible: !TcpCamera.isConnected
        anchors.fill: parent
        text: qsTr("Camera connecting...")
    }

    // 消息注册
    Connections {
        target: TcpCamera
        onMsg: {
//            console.log(str)
            messagebox.showMsg(str)
        }

        onCaptureFinished: {

        }

        onConnectStatusChanged: {
            if( stackView.depth > 1 && !TcpCamera.isConnected ) {
                stackView.pop()
            }
        }
    }

    MessageBox {
        id: messagebox
        z: 100
    }
}
