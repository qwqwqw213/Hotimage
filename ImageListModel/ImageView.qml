import QtQuick 2.0
import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.5

Item {
    visible: true
    anchors.fill: parent

    Rectangle {
        id: title
        x: 0
        y: 0
        width: parent.width
        height: 60
        color: "red"
    }

    SwipeView {
        id: imageSwipe
        x: 0
        y: 60
        width: parent.width
        height: parent.height - 60
        interactive: false
        orientation: ListView.Horizontal

        ImageListView {}
        ImagePlayer {}

        onCurrentIndexChanged: {
//            currentItem.visible = true;
//            for(var i = 0; i < imageSwipe.count; i ++) {
//                if( i !== imageSwipe.currentIndex ) {
//                    imageSwipe.itemAt(i).visible = false
//                }
//            }
        }
    }
}
