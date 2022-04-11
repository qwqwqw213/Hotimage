import QtQuick 2.0
import QtQuick.Controls 2.14

AbstractButton {
    property alias label: label.text
    property real leftRightMargin: 30
    property real itemHeight: height * 0.55
    property int clickInterval: 0
    // 按钮点击时发送clicked信号
    // 再通过外部改变onOffStatus
    property bool onOffStatus: false
    width: parent.width

    Timer {
        id: timer
        interval: parent.clickInterval
        onTriggered: {
            parent.enabled = true
        }
    }
    onClicked: {
        enabled = false
        timer.start()
    }

    background: Rectangle {
        color: pressed ? "#606060" : "transparent"
    }

    Text {
        id: label
        color: "white"
        width: parent.width * 0.2 - anchors.leftMargin
        height: parent.itemHeight
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin : leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        verticalAlignment: Qt.AlignVCenter

//        Rectangle {
//            anchors.fill: parent
//            color: "transparent"
//            border.color: "red"
//            border.width: 1
//        }
    }

    // switch button
    Rectangle {
        id: btnSwitch

        width: 60
        height: 30
        anchors.right: parent.right
        anchors.rightMargin: Config.rightMargin > 0
                            ? Config.rightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        color: "#808080"
        radius: height / 2
        Rectangle {
            // background
            anchors.fill: parent
            color: "#00dd00"
            radius: height / 2
            opacity: (circle.x - circle.offStatus) / circle.onStatus
        }

        Rectangle {
            id: circle
            property real offStatus: parent.height / 2 - height / 2
            property real onStatus: parent.width - (parent.height / 2 - height / 2) - width

            width: parent.height * 0.85
            height: parent.height * 0.85
            x: onOffStatus ? onStatus : offStatus
            y: (parent.height - height) / 2
            radius: height / 2
            color: "white"
            Behavior on x {
                NumberAnimation {
                    duration: 130
                }
            }
        }
    }

    // 底部横条
    Rectangle {
        width: parent.width - label.anchors.leftMargin - btnSwitch.anchors.rightMargin
        height: 1
        anchors.left: label.left
        anchors.bottom: parent.bottom
        color: "#606060"
    }
}
