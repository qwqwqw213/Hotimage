QT += gui quick quickcontrols2 network

CONFIG += c++11

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
        AndroidInterface/androidinterface.cpp \
        Config/config.cpp \
        Function/ImageProvider/imageprovider.cpp \
        ImageListModel/VideoScanImage/videoscanimage.cpp \
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
    AndroidInterface/androidinterface.h \
    Config/config.h \
    Function/ImageProvider/imageprovider.h \
    ImageListModel/VideoScanImage/videoscanimage.h \
    ImageListModel/imagelistmodel.h \
    TcpCamera/tcpdef.h \
    TcpCamera/handshake.hpp \
    TcpCamera/tcpcamera.h \ \
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

QT += \
    sensors

message("ios lib")

HEADERS += \
    libs/ios/libavcodec/avcodec.h \
    libs/ios/libavformat/avformat.h \
    libs/ios/libavutil/avutil.h \
    libs/ios/libswscale/swscale.h \
    libs/ios/libswresample/swresample.h \
    Function/VideoProcess/videoprocess.h

SOURCES += \
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


QT += \
    androidextras \
    sensors

HEADERS += \
    libs/xtherm/thermometry.h \
    libs/ffmpeg/libavcodec/avcodec.h \
    libs/ffmpeg/libavformat/avformat.h \
    libs/ffmpeg/libavutil/avutil.h \
    libs/ffmpeg/libswresample/swresample.h \
    libs/ffmpeg/libswscale/swscale.h \
    Function/VideoProcess/videoprocess.h

SOURCES += \
    Function/VideoProcess/videoprocess.cpp


DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

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
    $$PWD/libs/android/$$ANDROID_ABIS/libswscale.so


INCLUDEPATH += $$PWD/libs/android
DEPENDPATH += $$PWD/libs/android

INCLUDEPATH += $$PWD/libs/android/ffmpeg
DEPENDPATH += $$PWD/libs/android/ffmpeg

DISTFILES += \
    android/src/org/qtproject/example/function.java
}

TRANSLATIONS += \
    translation/Cn.ts
