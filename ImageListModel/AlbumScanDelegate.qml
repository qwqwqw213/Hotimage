import QtQuick 2.11

Rectangle{
    id: wrapper
    property bool pinchAreaBool: true
    color: "transparent"

//    property real initWidth : Math.min(width,height)
//    property real initHeight: Math.max(width,height)
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


      onContentXChanged: {
          imagePlayer.hideTitle()
      }

      onContentYChanged: {
          imagePlayer.hideTitle()
      }

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
              resizeAnimation.running = false
              imagePlayer.hideTitle()
          }

          onPinchUpdated: {
              console.log(pinch.scale)

              // adjust content pos due to drag
              flick.contentX += pinch.previousCenter.x - pinch.center.x
              flick.contentY += pinch.previousCenter.y - pinch.center.y

              // resize content
              flick.resizeContent((initialWidth * pinch.scale) < imagePlayer.width ?
                                      imagePlayer.width : (initialWidth * pinch.scale),
                                  (initialHeight * pinch.scale) < imagePlayer.height ?
                                      imagePlayer.height : (initialHeight * pinch.scale), pinch.center)
          }

          onPinchFinished: {
              // Move its content within bounds.
              if( flick.contentWidth < initw || flick.contentHeight < inith ) {
                  console.log("pinch finished, resize")
                  resizeAnimation.run(0, 0, initw, inith)
              }
              else {
                  flick.returnToBounds()
              }
          }

          Image {
              id: image
              opacity: wrapper.opacity
              width: flick.contentWidth - 10
              height: flick.contentHeight - 10
              cache: true
              source: fileType == 0 ? path : ""
              anchors.centerIn: parent
              asynchronous: true
              fillMode: Image.PreserveAspectFit
          }

          MouseArea {
              Timer {
                  id: timer
                  interval: 100
                  onTriggered: {
                      imagePlayer.autoTitle()
                  }
              }

              function isValidClicked(x, y) {
                  var left = (image.width - image.paintedWidth) / 2
                  var right = left + image.paintedWidth
                  var top = (image.height - image.paintedHeight) / 2
                  var bottom = top + image.paintedHeight

                  if( x >= left && x <= right
                          && y >= top && y <= bottom ) {
                      return {
                          res: true,
                          x: x - image.width / 2,
                          y: y - image.height / 2
                      }
                  }
                  else {
                      return {
                          res: false,
                          x: 0,
                          y: 0
                      }
                  }
              }

              anchors.fill: parent
              onDoubleClicked: {
                  var ret = isValidClicked(mouseX, mouseY)
                  if( ret.res )
                  {
                      imagePlayer.hideTitle()

                      if( flick.contentWidth > initw
                              || flick.contentHeight > inith ) {
                          // zoom out
                          resizeAnimation.run(0, 0, initw, inith)
                      }
                      else {
                          // zoom in
                          var w = initw * 3
                          var h = inith * 3
                          var cx = (w - initw) / 2
                          var cy = (h - inith) / 2
                          var x = cx + ret.x * 3
                          if( x < 0 ) {
                              x = 0
                          }
                          if( x > (w - initw) ) {
                              x = w - initw
                          }

                          var y = cy + ret.y * 3
                          if( y < 0 ) {
                              y = 0;
                          }
                          if( y > (h - inith) ) {
                              y = inith
                          }

                          resizeAnimation.run(x, y, w, h)
                      }

                      timer.stop()
                  }
              }
              onClicked: {
                  wrapper.itemClicked()
                  if( fileType === 1 )
                  {
//                      ImageModel.openVideo(index)
                      imagePaintView.openStream(path)
                  }

                  timer.start();
              }
          }

          ParallelAnimation {
              id: resizeAnimation
              running: false
              loops: 1

              property real to_x
              property real to_y
              property real to_w
              property real to_h
              function run(x, y, w, h) {
                  to_x = x
                  to_y = y
                  to_w = w
                  to_h = h
                  running = true
              }

              PropertyAnimation {
                  target: flick
                  property: "contentWidth"
                  from: flick.contentWidth
                  to: resizeAnimation.to_w
                  duration: 200
                  easing.type: Easing.OutCubic
              }
              PropertyAnimation {
                  target: flick
                  property: "contentHeight"
                  from: flick.contentHeight
                  to: resizeAnimation.to_h
                  duration: 200
                  easing.type: Easing.OutCubic
              }
              PropertyAnimation {
                  target: flick
                  property: "contentX"
                  from: flick.contentX
                  to: resizeAnimation.to_x
                  duration: 200
                  easing.type: Easing.OutCubic
              }
              PropertyAnimation {
                  target: flick
                  property: "contentY"
                  from: flick.contentY
                  to: resizeAnimation.to_y
                  duration: 200
                  easing.type: Easing.OutCubic
              }
          }
      }
    }
}
