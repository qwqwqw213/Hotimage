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
//              visible: !VideoPlayer.playing
              width: flick.contentWidth - 10
              height: flick.contentHeight - 10
              cache: true
//              source: fileType == 0 ? path : ""
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
//                      if( imagePaintView.playing ) {
//                          imagePaintView.closeStream()
//                          imagePlayer.autoTitle()
//                      }
                      if( VideoPlayer.playing ) {
//                          VideoPlayer.closeStream()
//                          imagePlayer.autoTitle()
                          if( progressBar.opacity > 0 ) {
                              progressBar.opacity = 0
                          }
                          else {
                              progressBar.opacity = 1
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
                           (VideoPlayer.playing ? (VideoPlayer.playIndex === index ? false : true) : true)
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
                      if( !VideoPlayer.playing ) {
                          console.log("video play index:", index)
//                          imagePaintView.openStream(filePath, image.paintedWidth, image.paintedHeight)
                          VideoPlayer.openStream(filePath, image.paintedWidth, image.paintedHeight, index)
                          imagePlayer.hideTitle()
                          progressBar.opacity = 1
                      }
                  }
              }
          }

          // VideoView
          Image {
              visible: VideoPlayer.playing ?
                           ((VideoPlayer.playIndex === index) ? true : false)
                         : false
              width: flick.contentWidth - 10
              height: flick.contentHeight - 10
              source: VideoPlayer.playIndex === index ? VideoPlayer.frameUrl : ""
              anchors.centerIn: parent
              fillMode: Image.PreserveAspectFit

              // 滚动条
              Rectangle {
                  id: progressBar
                  width: parent.width * 0.85
                  height: 60
                  x: (parent.width - width) / 2.0
                  y: parent.height - height - 10
                  radius: 10
                  color: "#D0505050"

                  opacity: 0

                  Behavior on opacity {
                      OpacityAnimator {
                          duration: 200
                      }
                  }

                  MouseArea {
                      anchors.fill: parent
                  }

                  // 暂停按钮
                  Text {
                      id: btnPause
                      text: "\uf28b"
                      font.family: "FontAwesome"
                      font.pixelSize: parent.height * 0.75
                      anchors.left: parent.left
                      anchors.leftMargin: contentWidth / 2
                      anchors.verticalCenter: parent.verticalCenter
                      color: btnPauseArea.pressed ? "#a0a0a0" : "white"
                      MouseArea {
                          id: btnPauseArea
                          anchors.fill: parent
                          onClicked: {
      //                        console.log("video pause")
      //                        imagePaintView.closeStream()
                              VideoPlayer.closeStream()
                              imagePlayer.autoTitle()
                          }
                      }
                  }

                  // 进度条底条
                  Rectangle {
                      width: parent.width * 0.55
                      height: 6
                      x: parent.width - parent.width * 0.15 - width
                      anchors.verticalCenter: parent.verticalCenter
                      color: "#505050"
                      radius: 10

                      Text {
                          id: currentTime
                          anchors.right: parent.left
                          anchors.rightMargin: 10
                          anchors.verticalCenter: parent.verticalCenter
      //                    text: imagePaintView.currentTime
                          text: VideoPlayer.currentTime
                          font.pixelSize: parent.height * 2.5
                          color: "white"
                      }

                      Text {
                          id: totalTime
                          anchors.left: parent.right
                          anchors.leftMargin:  10
                          anchors.verticalCenter: parent.verticalCenter
      //                    text: imagePaintView.totalTime
                          text: VideoPlayer.totalTime
                          font.pixelSize: parent.height * 2.5
                          color: "white"
                      }

                      //  进度条
                      Rectangle {
                          id: bar
      //                    width: parent.width * imagePaintView.progress
                          width: parent.width * VideoPlayer.progress
                          height: parent.height
                          color: "white"
                          radius: 10
                      }

                      // 进度条球
                      Rectangle {
                          width: parent.height + 4
                          height: width
                          x: bar.width - width / 2
                          radius: 100
                          color: "white"
                          anchors.verticalCenter: parent.verticalCenter
                          border.width: 1
                          border.color: "#a0a0a0"
                      }

                      MouseArea {
                          property real pressW
                          property real pressX
                          width: parent.width
                          height: parent.height * 6
                          enabled: false
                          anchors.verticalCenter: parent.verticalCenter
                          onPressed: {
                              pressX = mouseX
                              bar.width = pressX
                              pressW = bar.width
                          }

                          onMouseXChanged: {
                              var w = pressW + mouseX - pressX
                              if( w < 0 ) {
                                  w = 0
                              }
                              if( w > width ) {
                                  w = width
                              }
                              bar.width = w
                          }

                          onReleased: {

                          }
                      }
                  }

                  signal drag(int time)
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
