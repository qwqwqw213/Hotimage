QT += widgets quick quickcontrols2

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
        ImageListModel/imagelistmodel.cpp \
        TcpCamera/tcpcamera.cpp \
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
    Function/MadgwickAHRS.hpp \
    ImageListModel/imagelistmodel.h \
    TcpCamera/tcpdef.h \
    TcpCamera/handshake.hpp \
    TcpCamera/tcpcamera.h \

DEFINES += ANDROID_APP

DEPENDPATH += $$PWD/Function
INCLUDEPATH += $$PWD/Function

greaterThan(QT_MAJOR_VERSION,4){
        TARGET_ARCH=$${QT_ARCH}
}else{
        TARGET_ARCH=$${QMAKE_HOST.arch}
}



android {

contains(TARGET_ARCH, arm64-v8a){
BUILD_LIB=arm64-v8a
} else {
BUILD_LIB=armeabi-v7a
}


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


message("build lib:" $$BUILD_LIB)

#LIBS += -L$$PWD/libs/armeabi-v7a/ \
#    -lthermometry \
#    -lavcodec \
#    -lavfilter \
#    -lavformat \
#    -lavutil \
#    -lswresample \
#    -lswscale

#ANDROID_EXTRA_LIBS = \
#    $$PWD/libs/armeabi-v7a/libthermometry.so \
#    $$PWD/libs/armeabi-v7a/libavcodec.so \
#    $$PWD/libs/armeabi-v7a/libavfilter.so \
#    $$PWD/libs/armeabi-v7a/libavformat.so \
#    $$PWD/libs/armeabi-v7a/libavutil.so \
#    $$PWD/libs/armeabi-v7a/libswresample.so \
#    $$PWD/libs/armeabi-v7a/libswscale.so

LIBS += -L$$PWD/libs/arm64-v8a/ \
    -lthermometry \
    -lavcodec \
    -lavfilter \
    -lavformat \
    -lavutil \
    -lswresample \
    -lswscale

ANDROID_EXTRA_LIBS = \
    $$PWD/libs/arm64-v8a/libthermometry.so \
    $$PWD/libs/arm64-v8a/libavcodec.so \
    $$PWD/libs/arm64-v8a/libavfilter.so \
    $$PWD/libs/arm64-v8a/libavformat.so \
    $$PWD/libs/arm64-v8a/libavutil.so \
    $$PWD/libs/arm64-v8a/libswresample.so \
    $$PWD/libs/arm64-v8a/libswscale.so


INCLUDEPATH += $$PWD/libs
DEPENDPATH += $$PWD/libs

INCLUDEPATH += $$PWD/libs/ffmpeg
DEPENDPATH += $$PWD/libs/ffmpeg

DISTFILES += \
    android/src/org/qtproject/example/function.java
}

TRANSLATIONS += \
    translation/Cn.ts
