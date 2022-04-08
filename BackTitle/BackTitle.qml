import QtQuick 2.0
import QtQuick.Controls 2.14

Rectangle {
    property alias titleText: title.text
    property alias buttonText: button.label

    signal buttonClicked()

    anchors.left: parent.left
    anchors.top: parent.top
    color: "#2f4f4f"

    MouseArea {
        anchors.fill: parent
    }

    Text {
        id: btnReturn
        width: parent.height - Config.topMargin
        height: width
        y: Config.topMargin

        anchors.left: parent.left
        font.family: Config.fontRegular
        font.pixelSize: width * 0.75
        text: "\uf104"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        color: btnReturnArea.pressed ? "#6f9f9f" : "white"
        MouseArea {
            id: btnReturnArea
            anchors.fill: parent
            onClicked: {
                stackView.pop()
            }
        }

        Text {
            id: title
            anchors.left: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Setting")
            color: "white"
        }
    }

    // 右侧按钮
    AbstractButton {
        id: button
        property alias label: label.text
        visible: label.text !== ""

        width: label.contentWidth + 20
        height: label.contentHeight + 20
        y: (parent.height - Config.topMargin - height) / 2 + Config.topMargin
        anchors.right: parent.right
        anchors.rightMargin: 30 + Config.rightMargin

        background: Rectangle {
            radius: height / 2
            color: button.pressed ? "#1f3f3f" : "#3f5f5f"
        }
        Text {
            id: label
            anchors.centerIn: parent
            color: "white"
        }
        onClicked: {
            parent.buttonClicked()
        }
    }
}
