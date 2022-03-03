import QtQuick 2.0
import QtQuick.Controls 2.14

AbstractButton {
    id: button
    property alias iconSource: icon.source
    property alias label: label.text
    property real leftRightMargin: 30
    property bool selection: false

    background: Rectangle {
        color: pressed ? "#606060" : "transparent"
    }

    // 图片图标
    Image {
        id: icon
        asynchronous: true
        width: parent.width > parent.height ? parent.height * 0.75 : parent.width * 0.75
        height: width
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
    }

    // 文本
    Text {
        id: label
        color: "white"
        anchors.left: icon.status !== Image.Null ?
                          icon.right : parent.left
        anchors.leftMargin: parent.leftRightMargin
        anchors.verticalCenter: icon.verticalCenter
    }

    // 选中图标
    Text {
        id: selectIcon
        anchors.right: parent.right
        anchors.rightMargin: Config.rightMargin > 0
                            ? Config.rightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        font.family: "FontAwesome"
        font.pixelSize: parent.leftRightMargin
        text: "\uf192"
        color: selection ?
                   "#ffffff" : "#505050"
        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
    }


    // 底部横条
    Rectangle {
        width: parent.width - label.x - (parent.width - (selectIcon.x + selectIcon.width))
        height: 1
        anchors.left: label.left
        anchors.bottom: parent.bottom
        color: "#606060"
    }
}
