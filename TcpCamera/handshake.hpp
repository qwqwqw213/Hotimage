#ifndef __HANDSHAKE_H__
#define __HANDSHAKE_H__

#define XTHERM_N16_MODE                 0x8004
#define XTHERM_YUYV_MODE                0x8005

#include "HandShakeDef.h"

#include "QString"
#include "QByteArray"

class HandShakeApi {

public:
    HandShakeApi() { }
    ~HandShakeApi() { }

    // 握手初始化
    inline QByteArray handshakeInit(const float &emiss,
                                    const float &reflected,
                                    const float &ambient,
                                    const float &humidness,
                                    const float &correction,
                                    const unsigned short &distance,
                                    const unsigned char &infraredState,
                                    const QString &ssid,
                                    const QString &password) {
        hs::t_packet p = initPacket(hs::__req_init);
        p.emiss = emiss;
        p.reflected = reflected;
        p.ambient = ambient;
        p.humidness = humidness;
        p.correction = correction;
        p.distance = distance;

        p.infraredState = infraredState;

        QByteArray _ssid = ssid.toUtf8();
        QByteArray _password = password.toUtf8();
        memcpy(p.hotspotSSID, _ssid.data(), _ssid.size());
        p.hotspotSSID[_ssid.size()] = '\0';
        memcpy(p.hotspotPassword, _password.data(), _password.size());
        p.hotspotPassword[_password.size()] = '\0';
        return packet(&p);
    }

    // 请求帧数据
    inline QByteArray requestFrame() {
        hs::t_packet p = initPacket(hs::__req_frame);
        return packet(&p);
    }

    // 设置xtherm摄像头参数
    inline QByteArray setCameraParam(const float &emiss,
                                     const float &reflected,
                                     const float &ambient,
                                     const float &humidness,
                                     const float &correction,
                                     const unsigned short &distance) {

        hs::t_packet p = initPacket(hs::__req_camera_param);
        p.emiss = emiss;
        p.reflected = reflected;
        p.ambient = ambient;
        p.humidness = humidness;
        p.correction = correction;
        p.distance = distance;
        return packet(&p);
    }

    // 刷新快门
    inline QByteArray shutter() {
        hs::t_packet p = initPacket(hs::__req_shutter);
        return packet(&p);
    }

    // 设置热点参数
    inline QByteArray setHotspot(const QString &ssid, const QString &password) {
        hs::t_packet p = initPacket(hs::__req_hotspot_info);
        QByteArray _ssid = ssid.toUtf8();
        QByteArray _password = password.toUtf8();
        memcpy(p.hotspotSSID, _ssid.data(), _ssid.size());
        p.hotspotSSID[_ssid.size()] = '\0';
        memcpy(p.hotspotPassword, _password.data(), _password.size());
        p.hotspotPassword[_password.size()] = '\0';
        return packet(&p);
    }

    // 打开红外线
    inline QByteArray setInfraedState(const uint8_t &state) {
        hs::t_packet p = initPacket(hs::__req_infrared);
        p.infraredState = state;
        return packet(&p);
    }

    inline QByteArray packet(hs::t_packet *p) {
        QByteArray byte;
        byte.resize(HANDSHAKE_PACK_SIZE);
        memcpy(byte.data(), p, sizeof(hs::t_packet));
        return byte;
    }

    // 数据解码
    inline size_t decode(QByteArray &buf, hs::t_header *header, uint8_t *data) {
        int headerSize = sizeof (hs::t_header);
        memcpy(header, buf.data(), headerSize);

        if( !header->isValidHeader() ) {
            int i = 0;
            int size = buf.size();
            while (i < size)
            {
                char first = buf.at(i);
                if( first == 'T' )
                {
                    QByteArray head = buf.mid(i, headerSize);
                    if( header->isValidHeader(head.data()) ) {
                        return i;
                    }
                    else {
                        i ++;
                        continue;
                    }
                }
                else {
                    i ++;
                }
            }
            return i;
        }

        if( header->bufferLength > buf.size() ) {
            return 0;
        }

        data = reinterpret_cast<uint8_t *>(buf.data() + headerSize);

        return header->bufferLength;
    }

private:
    hs::t_packet initPacket(const hs::RequestType &req) {
        hs::t_packet p;
        memset(&p, '\0', sizeof (hs::t_packet));
        p.marker[0] = 'R';
        p.marker[1] = 'e';
        p.marker[2] = 'C';
        p.marker[3] = 'v';
        p.request = req;
        return p;
    }
};
#endif


// int byte2int(const char *buf, const int &byteSize) {
//     int value = 0;
//     for(int i = 0; i < byteSize; i ++) {
//         value = (value << 8) | (uint8_t)(buf[i] & 0xFF);
//     }
//     return value;
// }

// std::string int2byte(const int &value, const int &byteSize) {
//     std::string byte;
//     byte.resize(byteSize);
//     for(int i = 0; i < byteSize; i ++) {
//         byte[i] = (value >> ((byteSize - 1 - i) * 8)) & 0xFF;
//     }
//     return byte;
// }

// std::string float2byte(const float &f)
// {
//     std::string str = "";
//     uint8_t *p = (uint8_t *)(&f);
//     for(int i = 0; i < 4; i ++) {
//         uint8_t u8 = *p ++;
//         str.push_back(u8);
//     }
//     return str;
// }

// float byte2float(const std::string &str)
// {
//     float f;
//     uint8_t *p = (uint8_t *)(&f);
//     for(int i = 0; i < 4; i ++) {
//         p[i] = str.at(i);
//     }
//     return f;
// }

// float byte2float(const char *byte) {
//     float f;
//     uint8_t *p = (uint8_t *)(&f);
//     for(int i = 0; i < 4; i ++) {
//         p[i] = byte[i];
//     }
//     return f;
// }

// QByteArray qfloat2byte(const float &f)
// {
//     QByteArray byte;
//     uint8_t *p = (uint8_t *)(&f);
//     for(int i = 0; i < 4; i ++) {
//         uint8_t u8 = *p ++;
//         byte.push_back(u8);
//     }
//     return byte;
// }
// QByteArray qint2byte(const int &value, const int &byteSize) {
//     QByteArray byte;
//     byte.resize(byteSize);
//     for(int i = 0; i < byteSize; i ++) {
//         byte[i] = (value >> ((byteSize - 1 - i) * 8)) & 0xFF;
//     }
//     return byte;
// }

// QByteArray pack(const t_setting_page &t) {
//     QByteArray byte;
//     byte.resize(HANDSHAKE_PACK_SIZE);
//     byte[0] = PACK_HEAD;
//     byte[1] = t.type;
//     switch (t.type) {
//     case __handshake: {
//         byte[2] = t.palette;
//         byte[3] = t.mode;
//         byte.replace(4, 4, qfloat2byte(t.emiss));
//         byte.replace(8, 4, qfloat2byte(t.reflected));
//         byte.replace(12, 4, qfloat2byte(t.ambient));
//         byte.replace(16, 4, qfloat2byte(t.humidness));
//         byte.replace(20, 2, qint2byte(t.distance, 2));
//         byte.replace(22, 4, qfloat2byte(t.correction));
//         byte[26] = t.hotspotMode;
//         byte[27] = t.frameMode;
//         int offset = 28;
//         byte.replace(offset, t.hotspot_ssid.length(), t.hotspot_ssid.c_str());
//         offset = offset + t.hotspot_ssid.length();
//         byte[offset] = ';';
//         offset = offset + 1;
//         byte.replace(offset, t.hotspot_password.length(), t.hotspot_password.c_str());
//         offset = offset + t.hotspot_password.length();
//         byte[offset] = ';';
//         break;
//     }
//     case __palette: {
//         byte[2] = t.palette;
//     }
//         break;
//     case __config: {
//         byte.replace(2, 4, qfloat2byte(t.emiss));
//         byte.replace(6, 4, qfloat2byte(t.reflected));
//         byte.replace(10, 4, qfloat2byte(t.ambient));
//         byte.replace(14, 4, qfloat2byte(t.humidness));
//         byte.replace(18, 2, qint2byte(t.distance, 2));
//         byte.replace(20, 4, qfloat2byte(t.correction));
//     }
//         break;
//     case __mode: {
//         byte[2] = t.mode;
//     }
//         break;
//     case __frameMode: {
//         byte[2] = t.frameMode;
//     }
//         break;
//     case __hotspot_info: {
//         byte[2] = (!t.hotspotMode) & 0x01;
//         int offset = 3;
//         byte.replace(offset, t.hotspot_ssid.length(), t.hotspot_ssid.c_str());
//         offset = offset + t.hotspot_ssid.length();
//         byte[offset] = ';';
//         offset = offset + 1;
//         byte.replace(offset, t.hotspot_password.length(), t.hotspot_password.c_str());
//         offset = offset + t.hotspot_password.length();
//         byte[offset] = ';';
//     }
//         break;
//     default: break;
//     }
//     return byte;
// }
