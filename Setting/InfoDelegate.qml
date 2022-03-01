import QtQuick 2.0


Item {
    property alias label: label.text
    property alias value: value.text
    property real leftRightMargin: 30

    Text {
        id: label
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
    }

    Text {
        id: value
        color: "white"
        anchors.right: parent.right
        anchors.rightMargin: Config.rightMargin > 0
                            ? Config.rightMargin + parent.leftRightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        text: parent.value
    }

    // 底部横条
    Rectangle {
        width: parent.width - label.x - value.anchors.rightMargin
        height: 1
        anchors.left: label.left
        anchors.bottom: parent.bottom
        color: "#606060"
    }
}
