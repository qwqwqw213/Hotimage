#ifndef __HANDSHAKE_H__
#define __HANDSHAKE_H__

#define HANDSHAKE_PACK_SIZE             1490

#include "HandShakeDef.h"
#include "iostream"


#ifndef MOBILE_APP
#include "V4L2Control.hpp"

#endif

class HandShake
{
public:
#ifndef MOBILE_APP
    HandShake() {
        memset(&packet, 0, sizeof(hs::t_packet));
    }

    int readPacket(const char *buf, const int &fd) {
        hs::t_packet p;
        memcpy(&p, buf, sizeof(hs::t_packet));

        if( !p.isValidHeader() ) {
            // std::cout << "invalid packet \n";
            return hs::__req_invalid;
        }

        // std::cout << "req type: " << (int)p.request << "\n";
        switch (p.request)
        {
        case hs::__req_init: {
            // set camera param
            v4l2_ctl.sendEmissivity(fd, p.emiss);
            usleep(10000);
            v4l2_ctl.sendReflection(fd, p.reflected);
            usleep(10000);
            v4l2_ctl.sendAmb(fd, p.ambient);
            usleep(10000);
            v4l2_ctl.sendHumidity(fd, p.humidness);
            usleep(10000);
            v4l2_ctl.sendDistance(fd, p.distance);
            usleep(10000);
            v4l2_ctl.sendCorrection(fd, p.correction);

            // init hotspot enable
            std::string _ssid = p.hotspotSSID;
            std::string _password = p.hotspotPassword;
            if( !_ssid.empty() && !_password.empty() )
            {
                if( _ssid == packet.hotspotSSID
                    && _password == packet.hotspotPassword ) {
                    hotspot_enable = true;
                }
                else {
                    hotspot_enable = false;
                }
            }
            else {
                hotspot_enable = false;
            }

            std::cout << "- handshake init -" << "\n"
                      << "     frame format: " << (int)p.frameFormat << "\n"
                      << "            emiss: " << p.emiss << "\n"
                      << "        reflected: " << p.reflected << "\n"
                      << "          ambient: " << p.ambient << "\n"
                      << "        humidness: " << p.humidness << "\n"
                      << "         distance: " << p.distance << "\n"
                      << "       correction: " << p.correction << "\n"
                      << "   hotspot enable: " << hotspot_enable << "\n"
                      << "     hotspot ssid: " << _ssid << "\n"
                      << " hotspot password: " << _password << "\n";
        }
            break;
        case hs::__req_frame: {
            if( p.hotspotSSID == hotspot_ssid
                && p.hotspotPassword == hotspot_password ) {
                hotspot_enable = true;
            }
            else {
                hotspot_enable = false;
            }
        }
            break;
        case hs::__req_shutter: {
            std::cout << "refresh shutter \n";
            v4l2_ctl.v4l2Control(fd, CAM_SHUTTER);
        }
            break;
        case hs::__req_camera_param: {
            v4l2_ctl.sendEmissivity(fd, p.emiss);
            usleep(10000);
            v4l2_ctl.sendReflection(fd, p.reflected);
            usleep(10000);
            v4l2_ctl.sendAmb(fd, p.ambient);
            usleep(10000);
            v4l2_ctl.sendHumidity(fd, p.humidness);
            usleep(10000);
            v4l2_ctl.sendDistance(fd, p.distance);
            usleep(10000);
            v4l2_ctl.sendCorrection(fd, p.correction);

            std::cout << "- update camera param -" << "\n"
                      << "      emiss: " << p.emiss << "\n"
                      << "  reflected: " << p.reflected << "\n"
                      << "    ambient: " << p.ambient << "\n"
                      << "  humidness: " << p.humidness << "\n"
                      << "   distance: " << p.distance << "\n"
                      << " correction: " << p.correction << "\n";
        }
            break;
        case hs::__req_hotspot_info: {
            hotspot_ssid = p.hotspotSSID;
            hotspot_password = p.hotspotPassword;
            if( !hotspot_ssid.empty() && !hotspot_password.empty() ) {
                hotspot_enable = true;
            }
            else {
                hotspot_enable = false;
            }

            std::cout << "- update hotspot info -" << "\n"
                      << "    hotspot enable: " << hotspot_enable << "\n"
                      << "      hotspot_ssid: " << hotspot_ssid << "\n"
                      << "  hotspot_password: " << hotspot_password << "\n";
        }
            break;
        }

        memcpy(&packet, &p, sizeof(hs::t_packet));
        return p.request;
    }

    int frameFormat() { return packet.frameFormat; }
    bool hotspotEnable() { return hotspot_enable; }
    std::string hotspotSSID() { return hotspot_ssid; }
    std::string hotspotPassword() { return hotspot_password; }

private:
    XthermControl v4l2_ctl;
    hs::t_packet packet;
    bool hotspot_enable;
    std::string hotspot_ssid;
    std::string hotspot_password;
#endif
};


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


#endif

