import QtQuick 2.0
import QtQuick.Controls 2.14


Item {
    function append(str) {
        text.text += "\n"
        text.text += str
    }

    Flickable {
        anchors.fill: parent
        clip: true

        TextArea.flickable: TextArea {
            background: Rectangle {
                color: "#80303030"
            }
            padding: 10
            wrapMode: TextArea.Wrap
            color: "white"
            readOnly: true
        }
    }
}
