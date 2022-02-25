import QtQuick 2.0
import QtQuick.Controls 2.14

Popup {
    id: popup
    property alias text: label.text
    property real pw: parent.width
    property real ph: parent.height

    x: (pw - width) / 2
//    y: (height + 20) * opacity - height
    y: 0 - height

    width: label.width + 20
    height: label.height + 20

    opacity: (y + height) / (height + 20)
    background: Rectangle {
        color: "#D0505050"
        radius: 5
    }

    Behavior on y {
        enabled: popup.opening
        NumberAnimation {
            duration: 200
            onRunningChanged: {
                if( !running ) {
                    if( y > 0 ) {
                        hideTimer.start()
                    }
                    else {
                        popup.close()
                    }
                }
            }
        }
    }

//    Behavior on opacity {
//        NumberAnimation {
//            duration: 200
//            onRunningChanged: {
//                if( !running ) {
//                    if( opacity > 0 ) {
//                        hideTimer.start()
//                    }
//                    else {
//                        popup.close()
//                    }
//                }
//            }
//        }
//    }

    Timer {
        id: hideTimer
        interval: 3000
        onTriggered: {
//            popup.opacity = 0
            popup.y = 0 - popup.height
        }
    }

    property bool opening: false
    onOpened: {
        opening = true;
//        opacity = 1;
        popup.y = 20
    }
    onClosed: {
        opening = false
        label.text = ""
    }

    Text {
        id: label
        anchors.centerIn: parent
        color: "white"
        wrapMode: Text.Wrap

        function resize() {
            if( (label.contentWidth - 40) > popup.pw ) {
                label.width = popup.pw - 40
            }
        }

        onTextChanged: {
            if( text === "" ) {
                return
            }

            if( !popup.opening ) {
                resize()
                popup.open()
            }
            else {
                resize()
                hideTimer.restart()
            }
        }
    }
}
