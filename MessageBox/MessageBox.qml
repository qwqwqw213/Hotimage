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

    property real newOpacity: 1
    property real newY: 10
    property bool toshow: true
    property alias text: text.text
    property alias contentWidth: text.contentWidth
    property alias textW: text.width

    onVisibleChanged: {
        newOpacity = 1
        newY = 10
        toshow = true
        if( visible ) {
            animation.running = true
        }
    }

    function showMsg(text) {
        rect.text = text
        visible = true
        newOpacity = 1
        newY = 10
        toshow = true
        animation.running = true
    }

    Text {
        id: text
        opacity: rect.opacity
        font.pixelSize: 20
        color: "white"
        wrapMode: Text.Wrap
//        anchors.fill: parent
        width: parent.width - 20
        height: parent.height - 20
        anchors.centerIn: parent
        onContentWidthChanged: console.log(contentWidth, parent.width, width)
    }

    Timer {
        id: timer
        interval: 2000
        onTriggered: {
            rect.newY = 0 - rect.height
            rect.newOpacity = 0
            rect.toshow = false
            animation.running = true
        }
    }

    ParallelAnimation {
        id: animation
        YAnimator {
            target: rect
            from: rect.y
            to: rect.newY
            duration: 200
            easing.type: Easing.OutCurve
        }

        OpacityAnimator {
            target: rect
            from: rect.opacity
            to: rect.newOpacity
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
