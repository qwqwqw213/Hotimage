import QtQuick 2.0

Rectangle {
    property alias label: label.text
    color: "#2f4f4f"

    Text {
        id: label
        color: "white"
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0 ? Config.leftMargin : 20
        anchors.verticalCenter: parent.verticalCenter
    }
}
