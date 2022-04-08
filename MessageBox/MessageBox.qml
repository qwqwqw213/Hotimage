import QtQuick 2.0
import QtQuick.Controls 2.14

Popup {
    id: popup
    property alias text: label.text
    property real pw: parent.width
    property real ph: parent.height

    x: (pw - width) / 2
    y: opacity / 1 * (20 + Config.topMargin)

    width: label.width + 40
    height: label.height + 20

    opacity: 0
    background: Rectangle {
        color: "#D0505050"
        radius: 10
    }

    Behavior on opacity {
        enabled: popup.showing
        NumberAnimation {
            duration: 200
            onRunningChanged: {
                if( !running ) {
                    if( opacity > 0 ) {
                        hideTimer.start()
                    }
                    else {
                        popup.close()
                    }
                }
            }
        }
    }

    Timer {
        id: hideTimer
        interval: 3000
        onTriggered: {
            popup.opacity = 0
        }
    }

    property bool showing: false
    onOpened: {
        showing = true;
        opacity = 1;
    }
    onClosed: {
        showing = false
        label.text = ""
    }

    Text {
        id: label
        anchors.centerIn: parent
        color: "white"
        wrapMode: Text.Wrap
        font.pointSize: 15

        function resize() {
            if( (label.contentWidth - 80) > popup.pw ) {
                label.width = popup.pw - 80
            }
        }

        onTextChanged: {
            if( popup.showing ) {
                hideTimer.restart()
            }
        }

        onContentWidthChanged: {
            if( label.contentWidth < 1 ) {
                return
            }

            resize()
            if( !popup.showing ) {
                popup.open()
            }
            else {
                hideTimer.restart()
            }
        }
    }
}
