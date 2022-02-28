import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.Window 2.12
import QtQuick.Shapes 1.14
import Qt.labs.platform 1.1

import "./ImageListModel"
import "./Setting"
import "./Loading"
import "./MessageBox"
import "./ScaleBar"

ApplicationWindow {
    id: window
    visible: true

    minimumWidth: Config.width
    minimumHeight: Config.height

    /*
     *  mobile app full screen
     *  set visibility: Window.FullScreen
     *  win32
     *  set visibility: window.Minimized debugging
     */
    visibility: Config.isMobile ? Window.FullScreen : Window.Minimized

    title: qsTr("Hotimage")

    background: Rectangle {
        color: "black"
    }

//    onVisibleChanged: console.log("main visble status:", visible)
    onWidthChanged: console.log("main width:", width)
    onHeightChanged: console.log("main height:", height)

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

        property bool landscape: width > height ? true : false
        property bool videoPlay: true
        StackView.onDeactivated: {
            videoPlay = false
        }
        StackView.onActivating: {
            videoPlay = true
        }
        StackView.onActivated: {
//            PhoneApi.setRotationScreen(0)
            stackViewMouseArea.prevItem = null
        }

        // 顶部按钮栏
        Rectangle {
            id: topTool
            width: mainView.landscape ? parent.width * 0.1 : parent.width
            height: mainView.landscape ? parent.height : parent.height * 0.1
            color: "#90000000"
            z: 2

            property real safeWidth: (width - Config.leftMargin) * 0.8
            property real safeHeight: (height - Config.topMargin) * 0.8
            property real buttonSize: mainView.landscape ? safeWidth * 0.75 : safeHeight * 0.75

            // 温度开关
            Item {
                id: btnTempSetting
                width: parent.buttonSize
                height: parent.buttonSize
                x: (parent.width - width) / 2
                y: Config.topMargin + (parent.height - Config.topMargin - height) / 2
                visible: Config.canReadTemperature

                Text {
                    id: icon
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: parent.width * 0.8
                    text: "\uf2cb"
                    color: TcpCamera.showTemp ? "#dc143c" : "#696969"
                    rotation: window.oldRotation
                }

                MouseArea {
                    id: btnTempSettingArea
                    anchors.fill: parent
                    onClicked: {
                        TcpCamera.showTemp = !TcpCamera.showTemp
                    }
                }
            }

            // 快门刷新按钮
            Item {
                id: btnShutter
                width: parent.buttonSize
                height: parent.buttonSize
                anchors.top: btnTempSetting.top
                anchors.left: parent.left
                anchors.leftMargin: width / 2

                Text {
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: parent.width * 0.8
                    text: "\uf021"
                    color: btnShutterArea.pressed ? "#a0a0a0" : "white"
                    rotation: window.oldRotation
                }

                MouseArea {
                    id: btnShutterArea
                    anchors.fill: parent
                    onClicked: {
                        TcpCamera.shutter()
                    }
                }
            }

            // 设置按钮
            Item {
                id: btnSetting
                width: parent.buttonSize
                height: parent.buttonSize
                anchors.top: btnTempSetting.top
                anchors.right: parent.right
                anchors.rightMargin: width / 2

                Text {
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: parent.width * 0.8
                    text: "\uf013"
                    color: btnSettingArea.pressed ? "#a0a0a0" : "white"
                    rotation: window.oldRotation
                }

                MouseArea {
                    id: btnSettingArea
                    anchors.fill: parent
                    onClicked: {
                        stackView.push(setting)
//                        stackView.push("qrc:/Setting/Setting.qml")
                    }
                }
            }
        }

        // 摄像头图像
        Rectangle {
            id: cameraFrame
            anchors.left: parent.left
            anchors.top: topTool.bottom
            width: parent.width
            height: parent.height - (topTool.height + bottomTool.height)
            clip: true
            color: "black"

            Image {
                anchors.centerIn: parent
                width: parent.height * scaleBar.value
                height: parent.width * scaleBar.value
//                source: mainView.videoPlay ?
//                            (TcpCamera.isConnected ? TcpCamera.videoFrameUrl : "") : ""
                source: TcpCamera.isConnected ? TcpCamera.videoFrameUrl : ""
                rotation: 90

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
            }

            // 图像放大按钮
            ScaleBar {
                id: scaleBar
                width: parent.width
//                width: (parent.width - bottomTool.width - topTool.width) * 0.2
                height: parent.height * 0.2
                anchors.bottom: parent.bottom
                textRotation: window.oldRotation
            }
        }

        // 截图动画
        Rectangle {
            id: captureAnimationView
            visible: false
            color: "black"
            anchors.fill: parent
            z: 3

            OpacityAnimator on opacity {
                id: captureAnimation
                from: 1
                to: 0
                duration: 200

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
            width: mainView.landscape ? parent.width * 0.15 : parent.width
            height: mainView.landscape ? parent.height : parent.height * 0.15
            color: "#90000000"
            z: 2
            anchors.top: cameraFrame.bottom

            property real safeWidth: (width - Config.rightMargin) * 0.8
            property real safeHeight: (height - Config.bottomMargin) * 0.8
            property real buttonSize: mainView.landscape ? safeWidth * 0.75 : safeHeight * 0.75

            // 截图按钮
            Rectangle {
                id: btnCapture
                x: (parent.width - width) / 2
                y: (parent.height - Config.bottomMargin - height) / 2
                width: parent.buttonSize
                height: parent.buttonSize
                color: "transparent"

                border.color: btnCaptureArea.pressed ? "#A0A0A0" : "white"
                border.width: (width / 2) * 0.1
                radius: width / 2

                Rectangle {
                    id: btnCaptrueIcon
                    x: (parent.width - width) / 2
                    y: (parent.height - height) / 2
                    width: parent.width * 0.85
                    height: width
                    color: btnCaptureArea.pressed ? "#F01010" : "white"
                    radius: width / 2

                    Behavior on scale {
                        NumberAnimation {
                            duration: 200
                        }
                    }
                }

                MouseArea {
                    id: btnCaptureArea
                    anchors.fill: parent
                    pressAndHoldInterval: 100
                    onPressAndHold: {
                        btnCaptrueIcon.scale = 0.5
                    }
                    onReleased: {
                        btnCaptrueIcon.scale = 1.0
                        TcpCamera.capture()
                        captureAnimationView.start()
                    }
                }
            }

            // 相册按钮
            Item {
                id: btnPhoto
                width: parent.buttonSize * 0.85
                height: parent.buttonSize * 0.85
                anchors.verticalCenter: btnCapture.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: width / 2

                Behavior on scale {
                    NumberAnimation {
                        duration: 100
                    }
                }

                Image {
                    source: ImageModel.lastImagePath
                    width: btnPhoto.width * 0.9 * btnPhoto.scale
                    height: btnPhoto.height * 0.9 * btnPhoto.scale
                    anchors.centerIn: parent
                    rotation: window.oldRotation

                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        border.color: "white"
                        border.width: 1
                    }

                    Text {
                        visible: ImageModel.lastType === 1
                        anchors.centerIn: parent
                        font.family: "FontAwesome"
                        font.pixelSize: parent.width * 0.5
                        text: "\uf144"
                        color: "white"
                    }
                }

                MouseArea {
                    id: btnPhotoArea
                    anchors.fill: parent
                    onPressed: {
                        btnPhoto.scale = 0.95
                    }
                    onReleased: {
                        btnPhoto.scale = 1.0
                    }
                    onClicked: {
                        stackView.push(imageListView)
//                        imageListView.visible = true
                    }
                }

                Connections {
                    target: ImageModel
                    onAddNewFile: {
                        btnPhoto.scale = 0.95
                    }
                }
            }

            // 录像按钮
            Item {
                id: btnRecord
                width: parent.buttonSize * 0.85
                height: parent.buttonSize * 0.85
                anchors.verticalCenter: btnCapture.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: width / 2

                Text {
                    anchors.centerIn: parent
                    font.family: "FontAwesome"
                    font.pixelSize: btnRecord.width * 0.9
                    text: "\uf03d"
                    color: TcpCamera.encoding ? "red" : (btnRecordArea.pressed ? "#a0a0a0" : "white")
                    rotation: oldRotation
                }

                MouseArea {
                    id: btnRecordArea
                    anchors.fill: parent
                    onClicked: {
                        TcpCamera.openRecord()
                    }
                }
            }
        }
    }

    Setting {
        id: setting
    }

    ImageListView {
        id: imageListView
//        anchors.fill: parent
//        visible: false
//        y: parent.height
    }


    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainView

        property real hideX: 0 - width * 0.3

        focus: true
        Keys.onReleased: {
            if( event.key === Qt.Key_Back ) {
                if( stackView.depth > 1 ) {
                    stackView.pop()
                    event.accepted = true
                }
            }
        }

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
                to: stackView.hideX
                duration: 200
                easing.type: Easing.OutCurve
            }
        }
        popEnter: Transition {
            XAnimator {
                from: stackViewMouseArea.prevItem === null ?
                        stackView.hideX : stackViewMouseArea.prevItem.x
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

    MouseArea {
        id: stackViewMouseArea
        anchors.fill: parent

        enabled: stackView.depth > 1
        scrollGestureEnabled: false
//        propagateComposedEvents: true

        property bool pressValid: false
        property bool moveValid: false
        property real pressedX
        property real currentX
        property var prevItem: null

        onPressed: {
            if( mouseX < 10
                && mouseY > 60
                && stackView.depth > 1 )
            {
                pressValid = true;
                pressedX = mouseX
                currentX = stackView.currentItem.x
                prevItem = stackView.get(0)
            }
            else {
                mouse.accepted = false
            }
        }
        onPositionChanged: {
            if( pressValid && !moveValid ) {
                if( mouseX - pressedX > 20 ) {
                    moveValid = true
                }
            }
            if( moveValid ) {
                prevItem.visible = true
                var x = mouseX - pressedX + currentX
                if( x > stackView.currentItem.width ) {
                    x = width
                }
                if( x < 0 ) {
                    x = 0
                }

                var p = stackView.currentItem.x / stackView.currentItem.width
                stackView.currentItem.x = x
                prevItem.x = stackView.hideX - (stackView.hideX * p)
            }

        }
        onReleased: {
            if( pressValid ) {
                pressValid = false
                if( moveValid ) {
                    moveValid = false
                    stackView.pop()
                }
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
            messagebox.text = str
        }

        onCaptureFinished: {

        }

        onConnectStatusChanged: {
//            if( stackView.depth > 1 && !TcpCamera.isConnected ) {
//                stackView.pop()
//            }
        }
    }

    MessageBox {
        id: messagebox
        z: 100
    }
}
