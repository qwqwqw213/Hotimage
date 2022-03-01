import QtQuick 2.0
import QtQuick.Controls 2.14

AbstractButton {
    property alias iconSource: icon.text
    property alias label: label.text
    property real leftRightMargin: 30

    background: Rectangle {
        color: pressed ? "#606060" : "transparent"
    }

    // 文字图标
    Rectangle {
        id: iconBackground
        width: parent.width > parent.height ? parent.height * 0.75 : parent.width * 0.75
        height: width
        color: "#606060"
        radius: width * 0.15
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        Text {
            id: icon
            anchors.centerIn: parent
            font.family: "FontAwesome"
            font.pixelSize: parent.width * 0.55
            color: "white"
        }
    }

    Text {
        id: label
        color: "white"
        anchors.left: iconBackground.right
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
    }

    // icon
    Text {
        id: rightIcon
        anchors.right: parent.right
        anchors.rightMargin: Config.rightMargin > 0
                            ? Config.rightMargin + parent.leftRightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        font.family: "FontAwesome"
        font.pixelSize: parent.leftRightMargin
        text: "\uf105"
        color: "white"
    }

    // 底部横条
    Rectangle {
        width: parent.width
               - label.x
               - rightIcon.anchors.rightMargin
        height: 1
        anchors.left: label.left
        anchors.bottom: parent.bottom
        color: "#606060"
    }
}
