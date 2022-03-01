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
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin + parent.leftRightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
    }

    TextInput {
        id: value
        width: parent.width
               - parent.leftRightMargin * 2
               - (Config.leftMargin + Config.rightMargin)
               - parent.width * 0.3
        height: parent.height
        anchors.right: parent.right
        anchors.rightMargin: parent.leftRightMargin
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

    // 底部横条
    Rectangle {
        width: parent.width - label.anchors.leftMargin - value.anchors.rightMargin
        height: 1
        anchors.left: label.left
        anchors.bottom: parent.bottom
        color: "#606060"
    }
}
