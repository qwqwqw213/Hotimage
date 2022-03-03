import QtQuick 2.0
import QtQuick.Controls 2.14

AbstractButton {
    property alias label: label.text
    property alias value: value.text
    property real leftRightMargin: 30

    background: Rectangle {
        color: "transparent"
    }

    focus: true
    onClicked: value.forceActiveFocus()

    Text {
        id: label
        color: "white"
        width: parent.width * 0.3 - Config.leftMargin - parent.leftRightMargin
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
    }

    TextInput {
        id: value
//        width: parent.width
//               - parent.leftRightMargin * 2
//               - (Config.leftMargin + Config.rightMargin)
//               - parent.width * 0.3
        width: parent.width
               - (label.width + label.anchors.leftMargin)
               - (btnDelete.width + btnDelete.anchors.rightMargin)
               - 20
        height: parent.height
        anchors.left: label.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        clip: true
        color: "white"
        verticalAlignment: TextInput.AlignVCenter
        horizontalAlignment: TextInput.AlignRight
        selectByMouse: true

        onActiveFocusChanged: {
            if( activeFocus ) {
                cursorColor = "white"
            }
            else {
                cursorColor = "transparent"
            }
        }

        property color cursorColor: "transparent"
        Behavior on cursorColor {
            ColorAnimation {
                duration: 500
                onRunningChanged: {
                    if( !running && value.activeFocus ) {
                        if( Qt.colorEqual(value.cursorColor, "white") ) {
                            value.cursorColor = "transparent"
                        }
                        else {
                            value.cursorColor = "white"
                        }
                    }
                }
            }
        }
        cursorDelegate: Rectangle {
            id: cursor
            width: 1
            height: parent.contentHeight - 1
            color: parent.cursorColor
        }
    }

    // 删除按钮
    Text {
        id: btnDelete
        anchors.right: parent.right
        anchors.rightMargin: Config.rightMargin > 0
                            ? Config.rightMargin : parent.leftRightMargin
        width: value.contentHeight * 1.5
        height: width
        anchors.verticalCenter: parent.verticalCenter
        font.family: "FontAwesome"
        font.pixelSize: height
        text: "\uf056"
        color: btnDeleteArea.pressed ?
                   "#ffffff" : "#505050"
        verticalAlignment: Qt.AlignVCenter
        horizontalAlignment: Qt.AlignHCenter

        MouseArea {
            id: btnDeleteArea
            anchors.fill: parent
            focus: true
            onClicked: {
                console.log("delete clicked:", value.text)
                value.text = ""
            }
        }
    }

    // 底部横条
    Rectangle {
        width: parent.width - label.anchors.leftMargin - btnDelete.anchors.rightMargin
        height: 1
        anchors.left: label.left
        anchors.bottom: parent.bottom
        color: "#606060"
    }
}
