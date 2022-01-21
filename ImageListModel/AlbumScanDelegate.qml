import QtQuick 2.11

Item {
    id: wrapper
    property bool pinchAreaBool: true
//    color: "transparent"

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

    property alias contentX: flick.contentX
    property alias contentY: flick.contentY
    property alias contentW: flick.contentWidth
    property alias contentH: flick.contentHeight
    property alias paintW: image.paintedWidth
    property alias paintH: image.paintedHeight
    property alias source: image.source

    Flickable {
      id: flick
      anchors.fill: parent
      contentWidth: initw
      contentHeight: inith
      z: 1

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
          enabled: fileType === 0
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
              width: flick.contentWidth
              height: flick.contentHeight
              source: path
              anchors.centerIn: parent
              asynchronous: true
              fillMode: Image.PreserveAspectFit
          }

          MouseArea {
              id: mouseArea
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
                  if( ret.res && fileType === 0 )
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
                      if( VideoPlayer.playing > 0 )
                      {
                          if( videoViewLoader.progressBarOpacity > 0 ) {
                              videoViewLoader.progressBarOpacity = 0
                          }
                          else {
                              videoViewLoader.progressBarOpacity = 1
                          }
                      }
                      else {
                          timer.start();
                      }
                  }
                  else {
                      timer.start();
                  }
              }
          }

          Text {
              id: btnVideoPlay
              visible: fileType === 1 ?
                           (VideoPlayer.playing > 0 ? (VideoPlayer.playIndex === index ? false : true) : true)
                         : false
              anchors.centerIn: parent
              font.family: "FontAwesome"
              font.pixelSize: parent.width > parent.height ? parent.width * 0.15 : parent.height * 0.15
              text: "\uf144"
              color: btnVideoPlayArea.pressed ? "#a0a0a0" : "white"

              MouseArea {
                  id: btnVideoPlayArea
                  width: btnVideoPlay.contentWidth
                  height: btnVideoPlay.contentHeight
                  anchors.centerIn: parent
                  onClicked: {
                      if( VideoPlayer.playing < 1 ) {
                          console.log("video play index:", index)
//                          imagePaintView.openStream(filePath, image.paintedWidth, image.paintedHeight)
                          VideoPlayer.openStream(filePath, image.paintedWidth, image.paintedHeight, index)
                          imagePlayer.hideTitle()
                          videoViewLoader.progressBarOpacity = 1
                      }
                  }
              }
          }

          // VideoView
          Loader {
              id: videoViewLoader
              property real progressBarOpacity: 1

              active: VideoPlayer.playing > 0 ?
                          ((VideoPlayer.playIndex === index) ? true : false) : false
              anchors.centerIn: parent
              sourceComponent: videoView

              Component {
                  id: videoView
                  Image {
                      width: flick.contentWidth - 10
                      height: flick.contentHeight - 10
                      source: VideoPlayer.playIndex === index ? VideoPlayer.frameUrl : ""
                      anchors.centerIn: parent
                      fillMode: Image.PreserveAspectFit

                      // 视频顶部按钮
                      Rectangle {
                          id: btnVideoQuit
                          width: 50
                          height: 50
                          anchors.left: progressBarItem.left
                          anchors.top: parent.top
                          anchors.topMargin: 10
                          radius: 10
                          color: "#D0505050"
                          opacity: videoViewLoader.progressBarOpacity

                          Text {
                              id: btnVideoQuitIcon
                              font.family: "FontAwesome"
                              font.pixelSize: parent.height * 0.75
                              color: btnVideoQuitArea.pressed ? "#f0f0f0" : "white"
                              text: "\uf00d"
                              anchors.centerIn: parent
                              Behavior on scale {
                                  NumberAnimation {
                                      duration: 200
                                  }
                              }
                          }

                          MouseArea {
                              id: btnVideoQuitArea
                              anchors.fill: parent
                              onPressed: {
                                  btnVideoQuitIcon.scale = 0.75
                              }
                              onReleased: {
                                  btnVideoQuitIcon.scale = 1
                              }
                              onClicked: {
                                  // close video stream
                                  VideoPlayer.closeStream()
                              }
                          }
                      }

                      VideoProgressbar {
                          id: progressBarItem
                          width: parent.width * 0.85
                          height: Config.isLandscape ? 60 : 120
                          x: (parent.width - width) / 2.0
                          y: parent.height - height - 10
                          opacity: videoViewLoader.progressBarOpacity
                      }
                  }
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
