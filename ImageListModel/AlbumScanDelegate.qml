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
                          if( progressBarItem.opacity > 0 ) {
                              progressBarItem.opacity = 0
                          }
                          else {
                              progressBarItem.opacity = 1
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
                          progressBarItem.opacity = 1
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
                  opacity: progressBarItem.opacity

                  Text {
                      id: btnVideoQuitIcon
                      font.family: "FontAwesome"
                      font.pixelSize: parent.height * 0.75
                      color: btnVideoQuitArea.pressed ? "#f0f0f0" : "white"
                      text: "\uf00d"
                      anchors.centerIn: parent
                      Behavior on scale {
                          ScaleAnimator {
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

              // 视频进度条工具栏
              // 分成两个区域
              // 按纽栏
              // 进度条
              Rectangle {
                  id: progressBarItem

                  // 判断是否横屏
                  property bool isLandscape: parent.width > parent.height ?
                                                 true : false

                  width: parent.width * 0.85
                  height: isLandscape ? 60 : 120
                  x: (parent.width - width) / 2.0
                  y: parent.height - height - 10
                  radius: 10
                  color: "#D0505050"

                  opacity: 0

                  Behavior on opacity {
                      NumberAnimation {
                          duration: 200
                      }
                  }

                  MouseArea {
                      anchors.fill: parent
                  }

                  // 按钮栏
                  Item {
                      id: toolbar
                      width: parent.isLandscape ? parent.width * 0.25 : parent.width
                      height: parent.isLandscape ? parent.height : parent.height / 2
                      anchors.left: parent.left
                      anchors.bottom: parent.isLandscape ? progressbar.bottom : parent.bottom
                      // 暂停按钮
                      Text {
                          id: btnPause
                          text: VideoPlayer.playing === 1 ? "\uf04c" : "\uf04b"
                          font.family: "FontAwesome"
                          font.pixelSize: 45
                          anchors.centerIn: parent.isLandscape ?
                                                parent : undefined
                          anchors.horizontalCenter: parent.isLandscape ?
                                                        undefined : parent.horizontalCenter
                          color: btnPauseArea.pressed ? "#a0a0a0" : "white"
                          MouseArea {
                              id: btnPauseArea
                              anchors.fill: parent
                              onClicked: {
                                  VideoPlayer.pause()
                              }
                          }
                      }
                  }

                  // 进度条
                  Item {
                      id: progressbar
                      width: parent.isLandscape ? parent.width * 0.75 : parent.width
                      height: parent.isLandscape ? parent.height : parent.height / 2
                      anchors.right: parent.right

                      // 当前播放时间
                      Text {
                          id: currentTime
                          // 横屏布局
                          anchors.right: progressBarItem.isLandscape ?
                                             bottombar.left : undefined
                          anchors.rightMargin: progressBarItem.isLandscape ?
                                                   10 : 0
                          anchors.verticalCenter: progressBarItem.isLandscape ?
                                                      bottombar.verticalCenter : undefined
                          // 竖屏布局
                          anchors.left: progressBarItem.isLandscape ?
                                            undefined : bottombar.left
                          anchors.top: progressBarItem.isLandscape ?
                                           undefined : bottombar.bottom
                          anchors.topMargin: progressBarItem.isLandscape ?
                                           0 : 10
                          text: VideoPlayer.currentTime
                          font.pixelSize: 15
                          color: "white"
                      }

                      // 视频总时间
                      Text {
                          id: totalTime
                          // 竖屏布局
                          anchors.left: progressBarItem.isLandscape ?
                                            bottombar.right : undefined
                          anchors.leftMargin: progressBarItem.isLandscape ?
                                                  10 : 0
                          anchors.verticalCenter: progressBarItem.isLandscape ?
                                                      bottombar.verticalCenter : undefined

                          // 竖屏布局
                          anchors.right: progressBarItem.isLandscape ?
                                             undefined : bottombar.right
                          anchors.top: progressBarItem.isLandscape ?
                                           undefined : bottombar.bottom
                          anchors.topMargin: progressBarItem.isLandscape ?
                                           0 : 10
                          text: VideoPlayer.totalTime
                          font.pixelSize: 15
                          color: "white"
                      }

                      // 进度条底条
                      Rectangle {
                          id: bottombar
                          width: progressBarItem.isLandscape ?
                                     parent.width * 0.65 : parent.width * 0.85
                          height: 6
                          anchors.centerIn: parent
                          color: "#505050"
                          radius: 10

                          //  进度条
                          Rectangle {
                              id: rollbar
                              width: parent.width * VideoPlayer.progress
                              height: parent.height
                              color: "white"
                              radius: 10
                          }

                          // 进度条球
                          Rectangle {
                              width: parent.height + 6
                              height: width
                              x: rollbar.width
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
                                  rollbar.width = pressX
                                  pressW = rollbar.width
                              }

                              onMouseXChanged: {
                                  var w = pressW + mouseX - pressX
                                  if( w < 0 ) {
                                      w = 0
                                  }
                                  if( w > width ) {
                                      w = width
                                  }
                                  rollbar.width = w
                              }

                              onReleased: {

                              }
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
