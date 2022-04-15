#ifndef TCPDEF_H
#define TCPDEF_H

#include "handshake.hpp"
#include "QString"

#ifdef Q_OS_ANDROID
#include "QAndroidJniObject"
#include "QtAndroid"

#define JAVA_PATH       "org/qtproject/example/Function"
#endif

#ifndef Q_OS_WIN32

#define CAMERA_LEN      130
#define RANGE_MODE      120
#define SHUTTER_FIX     0
#define OUTPUT_MODE     5

#endif

#define SERVER_IP       "192.168.20.22"
#define SERVER_PORT     27015

typedef struct
{
    QString ip;
    int port;
    t_setting_page set;
    t_header header;
} tcp_config;

#endif // TCPDEF_H
