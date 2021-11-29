#ifndef __HANDSHAKE_H__
#define __HANDSHAKE_H__

#define HANDSHAKE_PAGE_SIZE             6
#define PAGE_HEAD                       0x30
#define DECODE_PAGE_SIZE                1490

#ifndef ANDROID_APP

#define CAM_SHUTTER         0x8000
#define CAM_PALETTE         0x8800

#include "Drive_Socket.hpp"
#include "Drive_V4L2Reader.hpp"

class XthermControl
{
public:
    int v4l2Control(int CameraFD, int value)
    {
        v4l2_control ctrl;
        ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
        ctrl.value = value;
        if (ioctl(CameraFD, VIDIOC_S_CTRL, &ctrl) == -1)
        {
            printf("v4l2_control error\n");
            return 0;
        }
        return 1;
    }

    void sendFloatCommand(int CameraFD, int position, unsigned char value0, unsigned char value1, unsigned char value2, unsigned char value3, int interval0,
                        int interval1, int interval2, int interval3, int interval4)
    {
        int psitionAndValue0 = (position << 8) | (0x000000ff & value0);
        // printf("psitionAndValue0:%X\n", psitionAndValue0);
        if (v4l2Control(CameraFD, psitionAndValue0) == 0)
        {
            // printf("control fail psitionAndValue0~~\n");
            // exit(EXIT_FAILURE);
        }
        int psitionAndValue1 = ((position + 1) << 8) | (0x000000ff & value1);
        // printf("psitionAndValue1:%X\n", psitionAndValue1);
        if (v4l2Control(CameraFD, psitionAndValue1) == 0)
        {
            // printf("control fail psitionAndValue1~~\n");
            // exit(EXIT_FAILURE);
        }
        int psitionAndValue2 = ((position + 2) << 8) | (0x000000ff & value2);
        // printf("psitionAndValue2:%X\n", psitionAndValue2);
        if (v4l2Control(CameraFD, psitionAndValue2) == 0)
        {
            // printf("control fail psitionAndValue2~~\n");
            // exit(EXIT_FAILURE);
        }
        int psitionAndValue3 = ((position + 3) << 8) | (0x000000ff & value3);
        // printf("psitionAndValue3:%X\n", psitionAndValue3);
        if (v4l2Control(CameraFD, psitionAndValue3) == 0)
        {
            // printf("control fail psitionAndValue3~~\n");
            // exit(EXIT_FAILURE);
        }
    }

    void sendUshortCommand(int CameraFD, int position, unsigned char value0, unsigned char value1)
    {
        int psitionAndValue0 = (position << 8) | (0x000000ff & value0);
        // printf("psitionAndValue0:%X\n", psitionAndValue0);
        if (v4l2Control(CameraFD, psitionAndValue0) == 0)
        {
            // printf("control fail psitionAndValue0~~\n");
            // exit(EXIT_FAILURE);
        }
        int psitionAndValue1 = ((position + 1) << 8) | (0x000000ff & value1);
        // printf("psitionAndValue1:%X\n", psitionAndValue1);
        if (v4l2Control(CameraFD, psitionAndValue1) == 0)
        {
            // printf("control fail psitionAndValue1~~\n");
            // exit(EXIT_FAILURE);
        }
    }

    void sendByteCommand(int CameraFD, int position, unsigned char value0, int interval0)
    {
        int psitionAndValue0 = (position << 8) | (0x000000ff & value0);
        v4l2Control(CameraFD, psitionAndValue0);
    }
    //温度偏移
    void sendCorrection(int CameraFD, float correction)
    {
        unsigned char iputCo[4];
        memcpy(iputCo, &correction, sizeof(float));
        sendFloatCommand(CameraFD, 0 * 4, iputCo[0], iputCo[1], iputCo[2], iputCo[3], 20, 40, 60, 80, 120);
        // printf("sendCorrection 0:%d,1:%d,2:%d,3:%d\n", iputCo[0], iputCo[1], iputCo[2], iputCo[3]);
    }
    //反射温度
    void sendReflection(int CameraFD, float reflection)
    {
        unsigned char iputRe[4];
        memcpy(iputRe, &reflection, sizeof(float));
        sendFloatCommand(CameraFD, 1 * 4, iputRe[0], iputRe[1], iputRe[2], iputRe[3], 20, 40, 60, 80, 120);
    }
    //环境温度
    void sendAmb(int CameraFD, float amb)
    {
        unsigned char iputAm[4];
        memcpy(iputAm, &amb, sizeof(float));
        sendFloatCommand(CameraFD, 2 * 4, iputAm[0], iputAm[1], iputAm[2], iputAm[3], 20, 40, 60, 80, 120);
    }
    //环境湿度
    void sendHumidity(int CameraFD, float humidity)
    {
        unsigned char iputHu[4];
        memcpy(iputHu, &humidity, sizeof(float));
        sendFloatCommand(CameraFD, 3 * 4, iputHu[0], iputHu[1], iputHu[2], iputHu[3], 20, 40, 60, 80, 120);
    }
    //发射率
    void sendEmissivity(int CameraFD, float emiss)
    {
        unsigned char iputEm[4];
        memcpy(iputEm, &emiss, sizeof(float));
        sendFloatCommand(CameraFD, 4 * 4, iputEm[0], iputEm[1], iputEm[2], iputEm[3], 20, 40, 60, 80, 120);
    }
    //距离
    void sendDistance(int CameraFD, unsigned short distance)
    {
        unsigned char iputDi[2];
        memcpy(iputDi, &distance, sizeof(unsigned short));
        sendUshortCommand(CameraFD, 5 * 4, iputDi[0], iputDi[1]);
    }
};
#else
#include "QDebug"
#include "QByteArray"

typedef struct
{
    uint8_t type;
    uint8_t palette;
    float emiss;
    float reflected;
    float ambient;
    float humidness;
    unsigned short distance;
    float correction;
} t_setting_page;
#endif

enum PixelFormat
{
    __yuyv = 0,
    __yuv420,
};

typedef struct
{
    int w;
    int h;
    PixelFormat format;
} t_handshake_page;

class HandShake
{
public:
enum RecvPageType
    {
        __handshake = 0x31,
        __shuffer,
        __palette,
        __config,
    };
    HandShake() { connected = false; }
    ~HandShake() { }

    bool isConnected() { return connected; }

#ifndef ANDROID_APP
    void wait() { isWait = true; }
    void reset(Socket *_s, const int _fd) {
        s = _s;
        fd = _fd;
        isWait = false;
        connected = false;
        canRecv = false;
        if( !t.joinable() )
        {
            exit = false;
            t = std::thread([=](){
            char *buf;
            while (!exit)
            {
                if( !isWait && (s != NULL) && canRecv )
                {
                    if( s->Recv(buf, DECODE_PAGE_SIZE) ) {
                        decode(buf);
                    }
                }
            }
        });
        }
    }
    void release() {
        exit = true;
        t.join();
    }

    void openRecv() { canRecv = true; }

    void disconnect() { canRecv = false; connected = false; }

    // 板子发送握手包
    // 长度 6
    std::string sendHandShake(const t_handshake_page &t) {
        std::string byte;
        byte += PAGE_HEAD;
        byte += int2byte(t.w, 2);
        byte += int2byte(t.h, 2);
        byte += (char)t.format;
        return byte;
    }
#endif

    // 高位在前
    int byte2int(const char *buf, const int &byteSize) {
        int value = 0;
        for(int i = 0; i < byteSize; i ++) {
            value = (value << 8) | (uint8_t)(buf[i] & 0xFF);
        }
        return value;
    }

    std::string int2byte(const int &value, const int &byteSize) {
        std::string byte;
        byte.resize(byteSize);
        for(int i = 0; i < byteSize; i ++) {
            byte[i] = (value >> ((byteSize - 1 - i) * 8)) & 0xFF;
        }
        return byte;
    }

    std::string float2byte(const float &f)
    {
        std::string str = "";
        uint8_t *p = (uint8_t *)(&f);
        for(int i = 0; i < 4; i ++) {
            uint8_t u8 = *p ++;
            str.push_back(u8);
        }
        return str;
    }

    float byte2float(const std::string &str)
    {
        float f;
        uint8_t *p = (uint8_t *)(&f);
        for(int i = 0; i < 4; i ++) {
            p[i] = str.at(i);
        }
        return f;
    }

    float byte2float(const char *byte) {
        float f;
        uint8_t *p = (uint8_t *)(&f);
        for(int i = 0; i < 4; i ++) {
            p[i] = byte[i];
        }
        return f;
    }

#ifdef ANDROID_APP
    QByteArray qfloat2byte(const float &f)
    {
        QByteArray byte;
        uint8_t *p = (uint8_t *)(&f);
        for(int i = 0; i < 4; i ++) {
            uint8_t u8 = *p ++;
            byte.push_back(u8);
        }
        return byte;
    }
    QByteArray qint2byte(const int &value, const int &byteSize) {
        QByteArray byte;
        byte.resize(byteSize);
        for(int i = 0; i < byteSize; i ++) {
            byte[i] = (value >> ((byteSize - 1 - i) * 8)) & 0xFF;
        }
        return byte;
    }

    int recvHandShake(const char *byte, const int &size, t_handshake_page *page) {
        if( size < HANDSHAKE_PAGE_SIZE ) {
            return 0;
        }

        if( byte[0] != PAGE_HEAD ) {
            // 无效包头
            // pop front byte
            return -1;
        }

        page->w = byte2int(&byte[1], 2);
        page->h = byte2int(&byte[3], 2);
        page->format = (PixelFormat)byte[5];
        connected = true;
        return HANDSHAKE_PAGE_SIZE;
    }

    QByteArray pack(const t_setting_page &t) {
        QByteArray byte;
        byte.resize(DECODE_PAGE_SIZE);
        byte[0] = PAGE_HEAD;
        byte[1] = t.type;
        switch (t.type) {
        case __handshake: {
            byte[2] = t.palette;
            byte.replace(3, 4, qfloat2byte(t.emiss));
            byte.replace(7, 4, qfloat2byte(t.reflected));
            byte.replace(11, 4, qfloat2byte(t.ambient));
            byte.replace(15, 4, qfloat2byte(t.humidness));
            byte.replace(19, 2, qint2byte(t.distance, 2));
            byte.replace(21, 4, qfloat2byte(t.correction));
            break;
        }
        case __palette: {
            byte[2] = t.palette;
        }
            break;
        case __config: {
            byte.replace(2, 4, qfloat2byte(t.emiss));
            byte.replace(6, 4, qfloat2byte(t.reflected));
            byte.replace(10, 4, qfloat2byte(t.ambient));
            byte.replace(14, 4, qfloat2byte(t.humidness));
            byte.replace(18, 2, qint2byte(t.distance, 2));
            byte.replace(20, 4, qfloat2byte(t.correction));
        }
            break;
        default: break;
        }
        return byte;
    }

    std::string s_pack(const t_setting_page &t)
    {
        std::string str;
        str += (uint8_t)PAGE_HEAD;
        str += t.type;
        switch (t.type) {
        case __handshake: {
            str += (uint8_t)t.palette;
            str += float2byte(t.emiss);
            str += float2byte(t.reflected);
            str += float2byte(t.ambient);
            str += float2byte(t.humidness);
            str += int2byte(t.distance, 2);
            str += float2byte(t.correction);
//            byte.replace(3, 4, qfloat2byte(t.emiss));
//            byte.replace(7, 4, qfloat2byte(t.reflected));
//            byte.replace(11, 4, qfloat2byte(t.ambient));
//            byte.replace(15, 4, qfloat2byte(t.humidness));
//            byte.replace(19, 2, qint2byte(t.distance, 2));
//            byte.replace(21, 4, qfloat2byte(t.correction));
            str += '0';
            break;
        }
        case __palette: {
//            byte[2] = t.palette;
        }
            break;
        case __config: {
//            byte.replace(2, 4, qfloat2byte(t.emiss));
//            byte.replace(6, 4, qfloat2byte(t.reflected));
//            byte.replace(10, 4, qfloat2byte(t.ambient));
//            byte.replace(14, 4, qfloat2byte(t.humidness));
//            byte.replace(18, 2, qint2byte(t.distance, 2));
//            byte.replace(20, 4, qfloat2byte(t.correction));
        }
            break;
        default: break;
        }
        return str;
    }
#endif

private:
    bool connected;
#ifndef ANDROID_APP
    Socket *s;
    std::thread t;
    bool exit;
    int fd;
    bool isWait;
    XthermControl ctl;
    bool canRecv;

    void decode(const char *buf) {
        if( buf )
        {
            std::string str(buf, DECODE_PAGE_SIZE);
            std::cout << "decode page: " << str << ", size:" << str.size() << "\n";
            if( buf[0] != PAGE_HEAD ) {
                std::cout << "invalid page\n";
                return;
            }
            switch(buf[1])
            {
            case HandShake::__handshake: {
                connected = true;
                ctl.v4l2Control(fd, CAM_PALETTE | buf[2]);

                float emiss = byte2float(&buf[3]);
                ctl.sendEmissivity(fd, emiss);
                float refltmp = byte2float(&buf[7]);
                ctl.sendReflection(fd, refltmp);
                float airtmp = byte2float(&buf[11]);
                ctl.sendAmb(fd, airtmp);
                float humi = byte2float(&buf[15]);
                ctl.sendHumidity(fd, humi);
                unsigned short distance = byte2int(&buf[19], 2);
                ctl.sendDistance(fd, distance);
                float fix = byte2float(&buf[21]);
                ctl.sendCorrection(fd, fix);
                std::cout << "handshake succrss, init param:\n"
                          << "palette" << (int)(buf[2]) << "\n"
                          << "emiss:" << emiss << "\n"
                          << "refltmp:" << refltmp << "\n"
                          << "airtmp:" << airtmp << "\n"
                          << "humi:" << humi << "\n"
                          << "fix:" << fix << "\n"
                          << "distance:" << distance << "\n";
            }
                break;
            case HandShake::__shuffer: {
                ctl.v4l2Control(fd, CAM_SHUTTER);
            }
                break;
            case HandShake::__palette: {
                ctl.v4l2Control(fd, CAM_PALETTE | buf[2]);
            }
                break;
            case HandShake::__config: {
                float emiss = byte2float(&buf[2]);
                ctl.sendEmissivity(fd, emiss);
                float refltmp = byte2float(&buf[6]);
                ctl.sendReflection(fd, refltmp);
                float airtmp = byte2float(&buf[10]);
                ctl.sendAmb(fd, airtmp);
                float humi = byte2float(&buf[14]);
                ctl.sendHumidity(fd, humi);
                unsigned short distance = byte2int(&buf[18], 2);
                ctl.sendDistance(fd, distance);
                float fix = byte2float(&buf[20]);
                ctl.sendCorrection(fd, fix);
                std::cout << "set param:\n"
                          << "emiss:" << emiss << "\n"
                          << "refltmp:" << refltmp << "\n"
                          << "airtmp:" << airtmp << "\n"
                          << "humi:" << humi << "\n"
                          << "fix:" << fix << "\n"
                          << "distance:" << distance << "\n";
            }
                break;
            default: break;
            }

        }
    }
#endif
};

#endif

