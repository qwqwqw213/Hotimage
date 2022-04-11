import QtQuick 2.0

Rectangle {
    property alias label: label.text
    property real leftRightMargin: 30
    color: "#2f4f4f"

    width: parent.width

    Text {
        id: label
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0 ? Config.leftMargin : leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
    }
}
