import QtQuick 2.12

Item {
    id: switchSelf

    property bool onOffStatus: false

    /*
     * 点击时先发送点击信号
     * 状态改变时, 才改变circle的坐标
     */
    signal clicked()

    Rectangle {
        color: Qt.rgba(100 / 255, 100 / 255, 100 / 255)
        radius: height / 2
        anchors.fill: parent
        Rectangle {
            // background
            anchors.fill: parent
            color: "#00dd00"
            radius: height / 2
            opacity: (circle.x - circle.offStatus) / circle.onStatus
        }

        Rectangle {
            id: circle
            property real offStatus: parent.height / 2 - height / 2
            property real onStatus: parent.width - (parent.height / 2 - height / 2) - width

            width: parent.height * 0.85
            height: parent.height * 0.85
            x: switchSelf.onOffStatus ? onStatus : offStatus
            y: (parent.height - height) / 2
            radius: height / 2
            color: "white"
            Behavior on x {
                NumberAnimation {
                    duration: 130
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
//                if( circle.x > circle.offStatus ) {
//                    circle.x = circle.offStatus
//                }
//                else {
//                    circle.x = circle.onStatus
//                }
                switchSelf.clicked()
            }
        }
    }
}
