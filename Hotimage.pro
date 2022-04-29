QT += gui quick quickcontrols2 network

CONFIG += c++11 \
          resources_big

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        CameraView/cameraview.cpp \
        Config/config.cpp \
        Function/ImageProvider/imageprovider.cpp \
        ImageListModel/imagelistmodel.cpp \
        TcpCamera/tcpcamera.cpp \
        VideoPlayer/videoplayer.cpp \
        main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    CameraView/cameraview.h \
    Config/config.h \
    Function/ImageProvider/imageprovider.h \
    Function/providercamera.hpp \
    Function/queue.hpp \
    ImageListModel/imagelistmodel.h \
    TcpCamera/HandShakeDef.h \
    TcpCamera/tcpdef.h \
    TcpCamera/handshake.hpp \
    TcpCamera/tcpcamera.h \ \
    TcpCamera/tcpsearcher.hpp \
    VideoPlayer/videoplayer.h

DEFINES += ANDROID_APP

DEPENDPATH += $$PWD/Function
INCLUDEPATH += $$PWD/Function

win32 {

greaterThan(QT_MAJOR_VERSION,4){
        TARGET_ARCH=$${QT_ARCH}
}else{
        TARGET_ARCH=$${QMAKE_HOST.arch}
}

contains(TARGET_ARCH, x86_64){
        message('64 bit function')
        BUILD_LIB=x64/
}else{
        message('32 bit function')
        BUILD_LIB=x86/
}

HEADERS += \
    libs/win32/libavcodec/avcodec.h \
    libs/win32/libavformat/avformat.h \
    libs/win32/libavutil/avutil.h \
    libs/win32/libswscale/swscale.h \
    Function/VideoProcess/videoprocess.h

SOURCES += \
    Function/VideoProcess/videoprocess.cpp

LIBS += -L$$PWD/libs/win32/$$BUILD_LIB \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale


INCLUDEPATH += $$PWD/libs/win32
DEPENDPATH += $$PWD/libs/win32
}

############################################################
##################### IOS config ###########################
############################################################
ios {

QMAKE_INFO_PLIST = $$PWD/IOSInterface/my.plist

app_launch_screen.files = $$files($$PWD/IOSInterface/iLaunchScreen.storyboard)
QMAKE_BUNDLE_DATA += app_launch_screen

QMAKE_ASSET_CATALOGS = $$PWD/resource/Images.xcassets
QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

OTHER_FILES += \
    $$PWD/IOSInterface/*.storyboard

QT += \
    sensors

message("ios lib")

HEADERS += \
    libs/ios/libavcodec/avcodec.h \
    libs/ios/libavformat/avformat.h \
    libs/ios/libavutil/avutil.h \
    libs/ios/libswscale/swscale.h \
    libs/ios/libswresample/swresample.h \
    IOSInterface/iosinterface.h \
    Function/VideoProcess/videoprocess.h

SOURCES += \
    IOSInterface/homeindicator.mm \
    IOSInterface/iosinterface.mm \
    Function/VideoProcess/videoprocess.cpp

LIBS += -L$$PWD/libs/ios \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale \
    -lswresample

LIBS += $$PWD/libs/ios/libbz2.1.0.tbd
LIBS += $$PWD/libs/ios/libiconv.2.tbd
LIBS += $$PWD/libs/ios/libz.1.tbd

LIBS += -F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks -framework VideoToolbox
LIBS += -F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks -framework CoreMedia
LIBS += -F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks -framework CoreVideo

INCLUDEPATH += $$PWD/libs/ios
DEPENDPATH += $$PWD/libs/ios
}


############################################################
################## android config ##########################
############################################################

android {

DEFINES  += \
    TEMPERATURE_SDK

QT += \
    androidextras \
    sensors

HEADERS += \
    AndroidInterface/androidinterface.h \
    Function/VideoProcess/videoprocess.h \
    libs/xtherm/thermometry.h \
    libs/ffmpeg/libavcodec/avcodec.h \
    libs/ffmpeg/libavformat/avformat.h \
    libs/ffmpeg/libavutil/avutil.h \
    libs/ffmpeg/libswresample/swresample.h \
    libs/ffmpeg/libswscale/swscale.h

SOURCES += \
    AndroidInterface/androidinterface.cpp \
    Function/VideoProcess/videoprocess.cpp

############## usb lib  ##############
HEADERS += \
        UVCamera/libusb/libusb/libusb.h \
        UVCamera/libusb/libusb/libusbi.h \
        UVCamera/libusb/libusb/os/events_posix.h \
        UVCamera/libusb/libusb/os/linux_usbfs.h \
        UVCamera/libusb/libusb/os/threads_posix.h \
        UVCamera/libusb/libusb/version.h \
        UVCamera/libusb/libusb/version_nano.h \
        UVCamera/androidUSBImp.hpp \
        UVCamera/uvcamera.h

SOURCES += \
        UVCamera/libusb/libusb/core.c \
        UVCamera/libusb/libusb/descriptor.c \
        UVCamera/libusb/libusb/hotplug.c \
        UVCamera/libusb/libusb/io.c \
        UVCamera/libusb/libusb/os/events_posix.c \
        UVCamera/libusb/libusb/os/linux_netlink.c \
        UVCamera/libusb/libusb/os/linux_usbfs.c \
        UVCamera/libusb/libusb/os/threads_posix.c \
        UVCamera/libusb/libusb/strerror.c \
        UVCamera/libusb/libusb/sync.c \
        UVCamera/libuvc/src/ctrl-gen.c \
        UVCamera/libuvc/src/ctrl.c \
        UVCamera/libuvc/src/device.c \
        UVCamera/libuvc/src/diag.c \
        UVCamera/libuvc/src/frame.c \
        UVCamera/libuvc/src/init.c \
        UVCamera/libuvc/src/misc.c \
        UVCamera/libuvc/src/stream.c \
        UVCamera/uvcamera.cpp

INCLUDEPATH += $$PWD/UVCamera/libusb
DEPENDPATH += $$PWD/UVCamera/libusb

INCLUDEPATH += $$PWD/UVCamera/libusb/libusb
DEPENDPATH += $$PWD/UVCamera/libusb/libusb

INCLUDEPATH += $$PWD/UVCamera/libusb/android
DEPENDPATH += $$PWD/UVCamera/libusb/android

INCLUDEPATH += $$PWD/UVCamera/libusb/os
DEPENDPATH += $$PWD/UVCamera/libusb/os

INCLUDEPATH += $$PWD/UVCamera/libuvc
DEPENDPATH += $$PWD/UVCamera/libuvc

INCLUDEPATH += $$PWD/UVCamera/libuvc/src
DEPENDPATH += $$PWD/UVCamera/libuvc/src

INCLUDEPATH += $$PWD/UVCamera/libuvc/include
DEPENDPATH += $$PWD/UVCamera/libuvc/include

INCLUDEPATH += $$PWD/UVCamera/libuvc/include/libuvc
DEPENDPATH += $$PWD/UVCamera/libuvc/include/libuvc
############## usb lib  ##############

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/xml/device_filter.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

contains(TARGET_ARCH, arm64-v8a){
BUILD_LIB=arm64-v8a
} else {
BUILD_LIB=armeabi-v7a
}
message("build lib:" $$BUILD_LIB $$ANDROID_ABIS)

LIBS += -L$$PWD/libs/android/$$ANDROID_ABIS/ \
    -lthermometry \
    -lavcodec \
    -lavfilter \
    -lavformat \
    -lavutil \
    -lswresample \
    -lswscale

ANDROID_EXTRA_LIBS = \
    $$PWD/libs/android/$$ANDROID_ABIS/libthermometry.so \
    $$PWD/libs/android/$$ANDROID_ABIS/libavcodec.so \
    $$PWD/libs/android/$$ANDROID_ABIS/libavfilter.so \
    $$PWD/libs/android/$$ANDROID_ABIS/libavformat.so \
    $$PWD/libs/android/$$ANDROID_ABIS/libavutil.so \
    $$PWD/libs/android/$$ANDROID_ABIS/libswresample.so \
    $$PWD/libs/android/$$ANDROID_ABIS/libswscale.so \
    $$PWD/UVCamera/lib/$$ANDROID_ABIS/libuvc.so \
    $$PWD/UVCamera/lib/$$ANDROID_ABIS/libjpeg-turbo1500.so \
    $$PWD/UVCamera/lib/$$ANDROID_ABIS/libusb100.so


INCLUDEPATH += $$PWD/libs/android
DEPENDPATH += $$PWD/libs/android

INCLUDEPATH += $$PWD/libs/android/ffmpeg
DEPENDPATH += $$PWD/libs/android/ffmpeg

DISTFILES += \
    android/src/org/qtproject/example/function.java
}

TRANSLATIONS += \
    translation/Cn.ts
