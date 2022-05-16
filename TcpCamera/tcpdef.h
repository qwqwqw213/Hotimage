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
//#define OUTPUT_MODE     5
#define N16_MODE        4
#define YUYV_MODE       5

#endif

#define SERVER_IP       "192.168.20.29"
#define SERVER_PORT     27016

typedef struct
{
    QString ip;
    int port;
    bool hotspotEnable;
    QString hotspotSsid;
    QString hotspotPassword;
    hs::t_packet set;
    hs::t_header header;
} tcp_config;

#endif // TCPDEF_H
