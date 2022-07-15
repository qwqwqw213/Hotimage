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
import "./FontButton"
import "./Log"

//import custom.cameraview 0.1

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

    property real oldRotation: Config.rotation
    Behavior on oldRotation {
        NumberAnimation {
            duration: 100
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

            Row {
                width: parent.width
                height: parent.height - Config.topMargin
                x: 0
                y: Config.topMargin

                property int buttonCount: Config.canReadTemperature ? 5 : 4

                leftPadding: 20
                rightPadding: leftPadding
                spacing: (width - leftPadding - rightPadding - (topTool.buttonSize * buttonCount)) / (buttonCount - 1)

                // 快门
                FontButton {
                    width: topTool.buttonSize
                    height: width
                    family: Config.fontSolid
                    icon: "\uf2f9"
                    color: pressed ? "#a0a0a0" : "white"
                    onClicked: TcpCamera.shutter()
                }

                // 对比度
                FontButton {
                    width: topTool.buttonSize
                    height: width
                    family: Config.fontRegular
                    icon: "\uf042"
                    color: TcpCamera.contrast > 1 ? "#dc143c" : "white"
                    onClicked: {
                        if( TcpCamera.contrast > 1 ) {
                            TcpCamera.contrast = 0
                        }
                        else {
                            TcpCamera.contrast = 2;
                        }
                    }
                }

                // 红外线
                FontButton {
                    width: topTool.buttonSize
                    height: width
                    family: Config.fontSolid
                    icon: "\uf0eb"
                    color: TcpCamera.infraredState > 0 ? "#dc143c" : "white"
                    onClicked: {
                        TcpCamera.infraredState = !TcpCamera.infraredState;
                    }
                }

                // 温度显示
                FontButton {
                    visible: Config.canReadTemperature
                    width: topTool.buttonSize
                    height: width
                    family: Config.fontLight
                    icon: "\uf2cb"
                    color: TcpCamera.showTemp ? "#dc143c" : "#696969"
                    onClicked: TcpCamera.showTemp = !TcpCamera.showTemp
                }

                // 设置跳转
                FontButton {
                    width: topTool.buttonSize
                    height: width
                    family: Config.fontLight
                    icon: "\uf013"
                    color: pressed ? "#a0a0a0" : "white"
                    onClicked: stackView.push("qrc:/Setting/Setting.qml")
                }
            }

            /*
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
                    font.family: Config.fontLight
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
                    font.family: Config.fontSolid
                    font.pixelSize: parent.width * 0.8
                    text: "\uf2f9"
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
                    font.family: Config.fontLight
                    font.pixelSize: parent.width * 0.8
                    text: "\uf013"
                    color: btnSettingArea.pressed ? "#a0a0a0" : "white"
                    rotation: window.oldRotation
                }

                MouseArea {
                    id: btnSettingArea
                    anchors.fill: parent
                    onClicked: {
//                        stackView.push(setting)
                        stackView.push("qrc:/Setting/Setting.qml")
                    }
                }
            }
            */
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
                source: mainView.videoPlay ? (TcpCamera.canReadUrl ? TcpCamera.frameUrl : "") : TcpCamera.freeze
                rotation: 270

                // 录像标志
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    font.family: Config.fontSolid
                    font.pixelSize: 30
                    text: "\uf03d"
                    color: "red"
                    visible: TcpCamera.encoding

                    // 录像时间
                    Text {
                        anchors.left: parent.right
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: parent.contentHeight
                        text: TcpCamera.recordTime
                        color: parent.color
                    }

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

            Loading {
                visible: mainView.videoPlay ? !TcpCamera.canReadUrl : false
                anchors.fill: parent
                text: qsTr("Camera connecting...")
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
            property real buttonSize: mainView.landscape ?
                                          safeWidth * 0.75
                                        : safeHeight * 0.75

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
                width: btnCapture.width * 0.75
                height: width
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

//                    Text {
//                        visible: ImageModel.lastType === 1
//                        anchors.centerIn: parent
//                        font.family: Config.fontLight
//                        font.pixelSize: parent.width * 0.5
//                        text: "\uf144"
//                        color: "white"
//                    }
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
//                        stackView.push(imageListView)
                        stackView.push("qrc:/ImageListModel/ImageListView.qml")
                    }
                }
            }

            // 录像按钮
            FontButton {
                id: btnRecord
                width: btnCapture.width * 0.75
                height: width
                anchors.verticalCenter: btnCapture.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: width / 2
                family: Config.fontSolid
                icon: "\uf03d"
//                pressColor: "#a0a0a0"
                color: pressed ? "#a0a0a0" : "white"
                onClicked: {
                    if( TcpCamera.encoding ) {
                        TcpCamera.closeRecord()
                    }
                    else {
                        TcpCamera.openRecord()
                    }
                }
                rotation: window.oldRotation
            }
        }
    }

//    Setting {
//        id: setting
//    }

//    ImageListView {
//        id: imageListView
////        anchors.fill: parent
////        visible: false
////        y: parent.height
//    }

    onClosing: {
        if( stackView.depth > 1 ) {
            close.accepted = false
            stackView.pop();
        }
    }


    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainView

        property real hideX: 0 - width * 0.3

        focus: true

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
                from: stackView.currentItem.x
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

        onPressed: (mouse) => {
            if( mouseX < 10
                && mouseY > 60
                && stackView.depth > 1 )
            {
                pressValid = true;
                pressedX = mouseX
                currentX = stackView.currentItem.x
                prevItem = stackView.get(stackView.depth - 2)
            }
            else {
                mouse.accepted = false
            }
        }
        onPositionChanged: {
            if( pressValid && !moveValid ) {
                if( mouseX - pressedX > 20 ) {
                    moveValid = true
                    if( stackView.depth < 3 ) {
                        mainView.videoPlay = true
                    }
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

//    Log {
//        id: log
//        anchors.left: parent.left
//        anchors.verticalCenter: parent.verticalCenter
//        width: parent.width / 2
//        height: parent.height * 0.65
//    }

    Connections {
        target: Config
        onUpdateLog: (str) => {
//            log.append(str)
        }
    }

    // 消息注册
    Connections {
        target: TcpCamera
        onMsg: (str) => {
            messagebox.text = str
        }

        onCaptureFinished: {

        }

        onConnectStatusChanged: {

        }
    }

    MessageBox {
        id: messagebox
        z: 100
    }

    Component.onCompleted: {
        Config.started()
    }
}
