import QtQuick 2.0
import QtQuick 2.14

import "../BackTitle"

Rectangle {
    color: "black"

    BackTitle {
        id: title
        width: parent.width
        height: 60 + Config.topMargin
        titleText: qsTr("Wireless Setting")
        buttonText: qsTr("Save")
        onButtonClicked: {
            var flag = TcpCamera.setWirelessParam(deviceIp.value, ssid.value, password.value)
            if( !flag ) {
                messagebox.text = qsTr("Invalid IPv4 format")
            }
            else {
                stackView.pop()
            }
        }
    }

    Flickable {
        anchors.left: title.left
        anchors.top: title.bottom
        width: parent.width
        height: parent.height - title.height
        clip: true
        contentHeight: column.implicitHeight
        bottomMargin: Config.bottomMargin

        Column {
            id: column
            anchors.fill: parent
            spacing: -1
            property real leftRightMargin: 30

            property real itemHeight: 60
            property real sectionHeight: 30

            SectionDelegate {
                width: parent.width
                height: parent.sectionHeight
                label: qsTr("Hotspot Info")
            }

            InputDelegate {
                id: ssid
                width: parent.width
                height: parent.itemHeight
                label: qsTr("SSID")
                value: TcpCamera.hotspotSSID
            }

            InputDelegate {
                id: password
                width: parent.width
                height: parent.itemHeight
                label: qsTr("Password")
                value: TcpCamera.hotspotPassword
            }

            Text {
                width: parent.width - parent.leftRightMargin * 2
                height: contentHeight + parent.leftRightMargin * 2
                anchors.left: parent.left
                anchors.leftMargin: parent.leftRightMargin
                topPadding: parent.leftRightMargin
                color: "white"
                wrapMode: Text.Wrap
                text: qsTr("If you want camera device connect to phone hotspot.")
                      + qsTr("\n1. fill in hotspot ssid and password, press \"Save\" button.")
                      + qsTr("\n2. open phone system setting, turn on hotspot switch.")
            }

            SectionDelegate {
                width: parent.width
                height: parent.sectionHeight
                label: qsTr("Camera Device")
            }

            InputDelegate {
                id: deviceIp
                width: parent.width
                height: parent.itemHeight
                label: qsTr("Device IP")
                value: TcpCamera.deviceIp
            }

            Text {
                width: parent.width - parent.leftRightMargin * 2
                height: contentHeight + parent.leftRightMargin * 2
                anchors.left: parent.left
                anchors.leftMargin: parent.leftRightMargin
                topPadding: parent.leftRightMargin
                color: "white"
                wrapMode: Text.Wrap
                text: qsTr("If camera device not connect. you can manual input the device ip and press \"Save\" button, will try connect input ip")
            }

            SectionDelegate {
                width: parent.width
                height: parent.sectionHeight
                label: qsTr("Local")
            }

            InfoDelegate {
                width: parent.width
                height: parent.itemHeight
                label: qsTr("Phone IP")
                value: TcpCamera.localIp
            }
        }
    }
}
