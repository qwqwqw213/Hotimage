import QtQuick 2.0
import QtQuick 2.14

Rectangle {
    id: rect
    anchors.horizontalCenter: parent.horizontalCenter
    width: text.contentWidth + 20 > parent.width ?
               parent.width - 20 : text.contentWidth + 20
    height: text.contentHeight + 20 > parent.height ?
                parent.width - 20 : text.contentHeight + 20
    y: 0 - height
    color: "#a0505050"
    radius: 5
    visible: false

    property real _opacity: 1
    property real _y: 10
    property bool toshow: true
    property alias text: text.text

    onVisibleChanged: {
        _opacity = 1
        _y = 10
        toshow = true
        if( visible ) {
            animation.running = true
        }
    }

    function showMsg(text) {
        rect.text = text
        rect.visible = true
        _opacity = 1
        _y = 10
        toshow = true
        animation.running = true
    }

    Text {
        id: text
        opacity: rect.opacity
        font.pixelSize: 20
        color: "white"
        text: qsTr("text rwerwerwer")
        wrapMode: Text.WordWrap
        anchors.centerIn: parent
        Component.onCompleted: console.log(contentWidth, contentHeight)
    }

    Timer {
        id: timer
        interval: 2000
        onTriggered: {
            rect._y = 0 - rect.height
            rect._opacity = 0
            rect.toshow = false
            animation.running = true
        }
    }

    ParallelAnimation {
        id: animation
        YAnimator {
            target: rect
            from: rect.y
            to: rect._y
            duration: 200
            easing.type: Easing.OutCurve
        }

        OpacityAnimator {
            target: rect
            from: rect.opacity
            to: rect._opacity
            duration: 200
            easing.type: Easing.OutCurve
        }

        onFinished: {
            if( toshow ) {
                timer.start()
            }
            else {
                rect.visible = false
            }
        }
    }


}
