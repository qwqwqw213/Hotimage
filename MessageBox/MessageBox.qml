import QtQuick 2.0
import QtQuick 2.14
import QtQuick.Controls 2.14

Popup {
    id: popup

    property real _w: parent.width
    property real _h: parent.height
    property real _opacity
    property real _y

    opacity: 0

    background: Rectangle {
        color: "#D0505050"
        radius: 5
    }

    property string state: "hide"
    function showMsg(show_text) {
        state = "show"
        text.text = show_text
    }

    onOpened: {
        animation.running = true
    }

    Text {
        id: text
        anchors.centerIn: parent
        color: "white"
        wrapMode: Text.Wrap
        font.pixelSize: 20

        onContentWidthChanged: {
            if( contentWidth + 20 > popup._w ) {
                popup.width = popup._w - 20
            }
            else {
                popup.width = contentWidth + 20
            }

            if( contentHeight + 20 > popup._h ) {
                popup.height = popup._h - 20
            }
            else {
                popup.height = contentHeight + 20
            }
            width = popup.width - 20
            height = popup.height - 20

            popup.x = (_w - width) / 2.0
            popup.y = 0 - height
            if( popup.state === "show" ) {
                popup._opacity = 1.0
                popup._y = 10
                popup.open()
            }
        }
    }


    ParallelAnimation {
        id: animation

        PropertyAnimation {
            target: popup
            property: "y"
            from: popup.y
            to: popup._y
            duration: 200
            easing.type: Easing.OutCurve
        }

        PropertyAnimation {
            target: popup
            property: "opacity"
            from: popup.opacity
            to: popup._opacity
            duration: 200
            easing.type: Easing.OutCurve
        }

        onFinished: {
            if( popup.state === "show" ) {
                timer.start()
            }
            else {
                text.text = ""
                popup.close()
            }
        }
    }

    Timer {
        id: timer
        interval: 2000
        onTriggered: {
            popup._y = 0 - popup.height
            popup._opacity = 0
            popup.state = "hide"
            animation.running = true
        }
    }


//    Rectangle {
//        id: rect
//        anchors.horizontalCenter: parent.horizontalCenter
//    //    width: text.contentWidth + 20 > parent.width ?
//    //               parent.width - 20 : text.contentWidth + 20
//    //    height: text.contentHeight + 20 > parent.height ?
//    //                parent.width - 20 : text.contentHeight + 20
//        y: 0 - height
//        color: "#a0505050"
//        radius: 5
//        visible: false
//        z: 999

//        property real newOpacity: 1
//        property real newY: 10
//        property bool toshow: true
//        property alias text: text.text

//        onVisibleChanged: {
//            newOpacity = 1
//            newY = 10
//            toshow = true
//            if( visible ) {
//                animation.running = true
//            }
//        }

//        Text {
//            id: text
//            opacity: rect.opacity
//            font.pixelSize: 20
//            color: "white"
//            wrapMode: Text.Wrap
//    //        anchors.fill: parent
//            anchors.centerIn: parent
//            onContentWidthChanged: {
//                if( contentWidth + 20 > rect.p_w ) {
//                    rect.width = rect.p_w - 20
//                }
//                else {
//                    rect.width = contentWidth + 20
//                }

//                if( contentHeight + 20 > rect.p_h ) {
//                    rect.height = rect.p_h - 20
//                }
//                else {
//                    rect.height = contentHeight + 20
//                }
//                width = rect.width - 20
//                height = rect.height - 20
//            }
//        }

//        Timer {
//            id: timer
//            interval: 2000
//            onTriggered: {
//                rect.newY = 0 - rect.height
//                rect.newOpacity = 0
//                rect.toshow = false
//                animation.running = true
//            }
//        }

//        ParallelAnimation {
//            id: animation
//            YAnimator {
//                target: rect
//                from: rect.y
//                to: rect.newY
//                duration: 200
//                easing.type: Easing.OutCurve
//            }

//            OpacityAnimator {
//                target: rect
//                from: rect.opacity
//                to: rect.newOpacity
//                duration: 200
//                easing.type: Easing.OutCurve
//            }

//            onFinished: {
//                if( toshow ) {
//                    timer.start()
//                }
//                else {
//                    rect.visible = false
//                }
//            }
//        }
//    }
}
