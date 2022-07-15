import QtQuick 2.0
import QtQuick.Controls 2.14

MouseArea {
    property string icon: label.text
    property alias family: label.font.family
    property alias tip: tip.text
    property bool selection: false
    property alias color: label.color

    property bool viceState: false
    property string viceIcon: ""

    Text {
        id: label
        anchors.fill: parent
        font.family: Config.fontLight
        font.pixelSize: parent.height
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        text: viceState ? viceIcon : icon
    }

    hoverEnabled: true
    property bool entered: false
    onEntered: {
        entered = true
    }
    onExited:  {
        entered = false
    }

    ToolTip {
        id: tip
        delay: 1000
        timeout: 3000
        visible: text !== "" ? parent.entered : false
        font.pixelSize: 15
        text: ""
    }

//    Rectangle {
//        color: "transparent"
//        anchors.fill: parent
//        border.color: "red"
//        border.width: 1
//    }
}
