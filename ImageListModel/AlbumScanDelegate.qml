import QtQuick 2.11

Rectangle{
    id: wrapper
    property bool pinchAreaBool: true
    color: "transparent"

    property real initWidth : Math.min(width,height)
    property real initHeight: Math.max(width,height)
    property int initw: width
    property int inith: height

    signal itemClicked

    onWidthChanged: {
        initw = width
        inith = height
        flick.contentWidth = initw
        flick.contentHeight = inith
    }
    onHeightChanged: {
        initw = width
        inith = height
        flick.contentWidth = initw
        flick.contentHeight = inith
    }


    Flickable {
      id: flick
      anchors.fill: parent
      anchors.centerIn: parent
      contentWidth: initw
      contentHeight: inith
      z:1

      PinchArea {
          id: pinchArea
          width: Math.max(flick.contentWidth, flick.width)
          height: Math.max(flick.contentHeight, flick.height)
          enabled: pinchAreaBool
          property real initialWidth
          property real initialHeight

          onPinchStarted: {
              initialWidth = flick.contentWidth
              initialHeight = flick.contentHeight
              console.log("onPinchStarted")
          }

          onPinchUpdated: {
              // adjust content pos due to drag
              flick.contentX += pinch.previousCenter.x - pinch.center.x
              flick.contentY += pinch.previousCenter.y - pinch.center.y

              // resize content
              flick.resizeContent((initialWidth * pinch.scale)< imagePlayer.width ?
                                      imagePlayer.width : (initialWidth * pinch.scale),
                                  (initialHeight * pinch.scale)< imagePlayer.height ?
                                      imagePlayer.height : (initialHeight * pinch.scale), pinch.center)
          }

          onPinchFinished: {
              // Move its content within bounds.
              console.log("onPinchFinished")
              flick.returnToBounds()
          }

          Image {
              opacity: wrapper.opacity
              width: flick.contentWidth - 10
              height: flick.contentHeight - 10
              cache: true
//              source: photoScan.model.folder + fileName
              source: fileType == 0 ? path : (ImageModel.videoPlaying ? ImageModel.videoFrameUrl : "")
              anchors.centerIn: parent
              asynchronous: true
              fillMode: Image.PreserveAspectFit
          }

          MouseArea {
              anchors.fill: parent
              onDoubleClicked: {
//                      flick.contentWidth = initw
//                      flick.contentHeight = inith
                  if( (flick.contentWidth > initw)
                          || (flick.contentHeight > initHeight) ) {
                      parallelAnimation.running = true;
                  }
              }
              onClicked: {
                  wrapper.itemClicked()
                  if( fileType === 1 ) {
//                      imagePlayer.playVideo(path)
                      ImageModel.openVideo(path)
                  }
              }

              property real pressedY
              property real pressedFlickY
              onPressed: {
                  pressedY = mouseY
                  pressedFlickY = flick.contentY
              }

              onMouseYChanged: {
//                      console.log("onMouseYChanged", pressedFlickY, pressedY, mouseY)
                  flick.contentY = pressedFlickY + pressedY - mouseY
              }
          }

          ParallelAnimation {
              id: parallelAnimation
              running: false
              loops: 1

              PropertyAnimation {
                  target: flick
                  property: "contentWidth"
                  from: flick.contentWidth
                  to: initw
                  duration: 300
                  easing.type: Easing.OutCubic
              }
              PropertyAnimation {
                  target: flick
                  property: "contentHeight"
                  from: flick.contentHeight
                  to: inith
                  duration: 300
                  easing.type: Easing.OutCubic
              }
              PropertyAnimation {
                  target: flick
                  property: "contentX"
                  from: flick.contentX
                  to: 0
                  duration: 300
                  easing.type: Easing.OutCubic
              }
              PropertyAnimation {
                  target: flick
                  property: "contentY"
                  from: flick.contentY
                  to: 0
                  duration: 300
                  easing.type: Easing.OutCubic
              }
          }
      }
    }
}
