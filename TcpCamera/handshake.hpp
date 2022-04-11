#ifndef __HANDSHAKE_H__
#define __HANDSHAKE_H__

#define PACK_HEAD                       0x30
#define HANDSHAKE_PACK_SIZE             1490

#define CAM_OUTPUT_NUC16_MODE           0x04
#define CAM_OUTPUT_YUYV_MODE            0x05


#include "HandShakeDef.h"

#ifndef ANDROID_APP

#define CAM_SHUTTER                     0x8000
#define CAM_PALETTE                     0x8800


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
    /*  数据包类型, see RecvPageType  */
    uint8_t type;
    /*  调色盘, 索引: 0 - 6  */
    uint8_t palette;
    /*
     *  04 整帧温度 输出nuc 16图像
     *  05 只有最大最小中心温度 输出yuyv图像
     */
    uint8_t mode;
    /*  模组设置参数
     *  发射率 \ 反射温度 \ 环境温度 \ 湿度 \ 距离 \ 修正值
     */
    float emiss;
    float reflected;
    float ambient;
    float humidness;
    unsigned short distance;
    float correction;

    // 0 连接路由器wi-fi
    // 1 路由器连接热点
    bool hotspotMode;

    std::string hotspot_ssid;
    std::string hotspot_password;
} t_setting_page;
#endif

typedef void (*HandShakeCallback)(const int &msg, void *content);

class HandShake
{
public:
    enum PackType {
        __handshake = 0x31,
        __shuffer,
        __palette,
        __config,
        __mode,
        __hotspot_info,
        __disconnect,
    };

    enum RequestType {
        __req_none = 0,
        __req_frame,
        __req_disconnect,
    };

    HandShake() { connected = false; }
    ~HandShake() { }

#ifndef ANDROID_APP
    void reset(Socket *_s, const int &_fd, HandShakeCallback func) {
        s = _s;
        fd = _fd;
        request = __req_none;
        msg_callback_func = func;
        if( !t.joinable() )
        {
            exit = false;
            t = std::thread([=](){
                char *buf;
                while (!exit)
                {
                    if( s->Recv(buf, HANDSHAKE_PACK_SIZE) ) {
                        decode(buf);
                    }
                    else {
                        usleep(1000 * 1000);
                    }
                }
                std::cout << "handshake thread release \n";
            });
        }
    }
    int hasRequest() {
        if( request > __req_none ) {
            int _req = request;
            request = __req_none;
            return _req;
        }
        return __req_none;
    }
    void release() {
        connected = false;
        exit = true;
        if( t.joinable() ) {
            t.join();
        }
    }

    int decode(const char *buf) {
        if( buf )
        {
            std::string str(buf, HANDSHAKE_PACK_SIZE);
            // std::cout << "decode page: " << str << ", size:" << str.size() << "\n";
            if( buf[0] != PACK_HEAD ) {
                // std::cout << "invalid page: " << str << "\n";
                // std::cout << "invalid page: " << (int)buf[0] << " , " << (int)buf[1] << "\n";
                return __req_none;
            }
            switch(buf[1])
            {
            case HandShake::__handshake: {
                if( !connected )
                {
                    connected = true;

                    int palette = buf[2];
                    int mode = buf[3];
                    ctl.v4l2Control(fd, 0x8000 | mode);
                    usleep(10000);
                    if( mode == 0x05 ) {
                        ctl.v4l2Control(fd, CAM_PALETTE | palette);
                    }

                    float emiss = byte2float(&buf[4]);
                    ctl.sendEmissivity(fd, emiss);
                    usleep(10000);
                    float refltmp = byte2float(&buf[8]);
                    ctl.sendReflection(fd, refltmp);
                    usleep(10000);
                    float airtmp = byte2float(&buf[12]);
                    ctl.sendAmb(fd, airtmp);
                    usleep(10000);
                    float humi = byte2float(&buf[16]);
                    ctl.sendHumidity(fd, humi);
                    usleep(10000);
                    unsigned short distance = byte2int(&buf[20], 2);
                    ctl.sendDistance(fd, distance);
                    usleep(10000);
                    float fix = byte2float(&buf[22]);
                    ctl.sendCorrection(fd, fix);
                    usleep(10000);
                    hotspot_mode = buf[26];
                    int start = 27;
                    size_t offset = str.find(';');
                    if( offset != std::string::npos
                        && str.find(';', offset) != std::string::npos )
                    {
                        hotspot_ssid = str.substr(start, offset - start);
                        offset += 1;
                        hotspot_password = str.substr(offset, str.find(';', offset) - offset);
                    }
                    std::cout << "- handshake succrss, init param -\n"
                            << "      palette: " << palette << "\n"
                            << "         mode: " << mode << "\n"
                            << "        emiss: " << emiss << "\n"
                            << "      refltmp: " << refltmp << "\n"
                            << "       airtmp: " << airtmp << "\n"
                            << "         humi: " << humi << "\n"
                            << "          fix: " << fix << "\n"
                            << "     distance: " << distance << "\n"
                            << " hotspot mode: " << hotspot_mode << "\n"
                            << "         ssid: " << hotspot_ssid << "\n"
                            << "     password: " << hotspot_password << "\n";
                }
                request = __req_frame;
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
            case HandShake::__hotspot_info: {
                hotspot_mode = buf[2];
                int start = 3;
                size_t offset = str.find(';');
                if( offset != std::string::npos
                    && str.find(';', offset) != std::string::npos )
                {
                    hotspot_ssid = str.substr(start, offset - start);
                    offset += 1;
                    hotspot_password = str.substr(offset, str.find(';', offset) - offset);
                    std::cout << "- hotspot info -\n"
                                << "          ssid: " << hotspot_ssid << "\n"
                                << "      password: " << hotspot_password << "\n";
                    if( msg_callback_func ) {
                        msg_callback_func(HandShake::__hotspot_info, this);
                    }

                }
                else {
                    std::cout << "cannot find hotspot info, socket data error \n";
                }
            }
                break;
            case HandShake::__disconnect: {
                request = __req_disconnect;
            }
                break;
            default: break;
            }
        }
        return request;
    }

    uint8_t hotspotMode() { return hotspot_mode; }
    std::string hotspotSSID() { return hotspot_ssid; }
    std::string hotspotPassword() { return hotspot_password; }
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

    QByteArray pack(const t_setting_page &t) {
        QByteArray byte;
        byte.resize(HANDSHAKE_PACK_SIZE);
        byte[0] = PACK_HEAD;
        byte[1] = t.type;
        switch (t.type) {
        case __handshake: {
            byte[2] = t.palette;
            byte[3] = t.mode;
            byte.replace(4, 4, qfloat2byte(t.emiss));
            byte.replace(8, 4, qfloat2byte(t.reflected));
            byte.replace(12, 4, qfloat2byte(t.ambient));
            byte.replace(16, 4, qfloat2byte(t.humidness));
            byte.replace(20, 2, qint2byte(t.distance, 2));
            byte.replace(22, 4, qfloat2byte(t.correction));
            byte[26] = t.hotspotMode;
            int offset = 27;
            byte.replace(offset, t.hotspot_ssid.length(), t.hotspot_ssid.c_str());
            offset = offset + t.hotspot_ssid.length();
            byte[offset] = ';';
            offset = offset + 1;
            byte.replace(offset, t.hotspot_password.length(), t.hotspot_password.c_str());
            offset = offset + t.hotspot_password.length();
            byte[offset] = ';';
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
        case __mode: {
            byte[2] = t.mode;
        }
            break;
        case __hotspot_info: {
            byte[2] = (!t.hotspotMode) & 0x01;
            int offset = 3;
            byte.replace(offset, t.hotspot_ssid.length(), t.hotspot_ssid.c_str());
            offset = offset + t.hotspot_ssid.length();
            byte[offset] = ';';
            offset = offset + 1;
            byte.replace(offset, t.hotspot_password.length(), t.hotspot_password.c_str());
            offset = offset + t.hotspot_password.length();
            byte[offset] = ';';
        }
            break;
        default: break;
        }
        return byte;
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
    uint8_t hotspot_mode;
    std::string hotspot_ssid;
    std::string hotspot_password;
    HandShakeCallback msg_callback_func;
    int request;
#endif
};

#endif

