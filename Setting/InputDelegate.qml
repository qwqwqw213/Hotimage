import QtQuick 2.0
import QtQuick.Controls 2.14

AbstractButton {
    property alias label: label.text
    property alias value: value.text
    property alias checkLabel: btnCheckText.text

    property real leftRightMargin: 20
    property real itemHeight: height * 0.65

    property string deleteFamily: Config.fontSolid
    property string deleteIcon: "\uf056"
    property alias readOnly: value.readOnly

    property bool onlyNumber: false
    property int numBottom: 30
    property int numTop: 65535

    signal checkClicked

    width: parent.width

    background: Rectangle {
        color: "transparent"
//        border.width: 1
//        border.color: "red"
    }

    focus: true
    onClicked: value.forceActiveFocus()

    Text {
        id: label
        color: "white"
        width: parent.width * 0.25 - anchors.leftMargin
        height: parent.itemHeight
        anchors.left: parent.left
        anchors.leftMargin: Config.leftMargin > 0
                            ? Config.leftMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        verticalAlignment: Qt.AlignVCenter
        clip: true

//        Rectangle {
//            anchors.fill: parent
//            color: "transparent"
//            border.color: "red"
//            border.width: 1
//        }
    }

    TextField {
        id: value
        background: Rectangle {
            color: "transparent"
//            border.color: "white"
//            border.width: 1
        }

        width: btnCheck.visible ?
                   parent.width
                   - (label.width + label.anchors.leftMargin)
                   - (btnCheck.width + btnCheck.anchors.rightMargin)
                   - 20
                 : parent.width
                   - (label.width + label.anchors.leftMargin)
                   - parent.leftRightMargin - 10
        height: parent.itemHeight
        anchors.left: label.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        color: "white"
        selectByMouse: true
        horizontalAlignment: TextInput.AlignRight
        verticalAlignment: TextInput.AlignVCenter
        rightPadding: btnDelete.width + 20

        validator: onlyNumber ? intValidator : null

        inputMethodHints: onlyNumber ? Qt.ImhDigitsOnly | Qt.ImhFormattedNumbersOnly : Qt.ImhNone

        onTextEdited: {
            if( onlyNumber ) {
                var num = Number(text)
                if( num < numBottom ) {
                    text = numBottom
                }
            }
        }

        // 删除按钮
        MouseArea {
            visible: !readOnly
            id: btnDelete
            width: parent.height * 0.5
            height: width
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            focus: false
            onClicked: {
                value.text = onlyNumber ? numBottom : ""
            }

//            Rectangle {
//                anchors.fill: parent
//                color: "transparent"
//                border.width: 1
//                border.color: "red"
//            }

            Text {
                id: btnDeleteIcon
                anchors.fill: parent
                font.family: deleteFamily
                font.pixelSize: width
                text: deleteIcon
                color: btnDelete.pressed ?
                           "#ffffff" : "#505050"
            }
        }
    }

    AbstractButton {
        id: btnCheck
        visible: btnCheckText.text == "" ? false : true
        width: parent.width * 0.15 - anchors.leftMargin
        height: parent.itemHeight
        anchors.right: parent.right
        anchors.rightMargin: Config.rightMargin > 0
                            ? Config.rightMargin : parent.leftRightMargin
        anchors.verticalCenter: parent.verticalCenter
        background: Rectangle {
            color: btnCheck.pressed ? "#416e6e" : "#606060"
            radius: height / 2
        }

        onClicked: parent.checkClicked()

        Text {
            id: btnCheckText
            anchors.centerIn: parent
            color: "white"
        }
    }

    // 底部横条
    Rectangle {
        width: parent.width - label.anchors.leftMargin - btnCheck.anchors.rightMargin
        height: 1
        anchors.left: label.left
        anchors.bottom: parent.bottom
        color: "#606060"
    }

    IntValidator {
        id: intValidator
        top: numTop
        bottom: numBottom
    }
}
