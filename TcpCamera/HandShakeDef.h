#ifndef __HANDSHAKEDEF_H__
#define __HANDSHAKEDEF_H__

#include "string"

#define HANDSHAKE_PACK_SIZE                 1490
#define HOTSPOT_SSID_LENGTH                 255
#define HOTSPOT_PASSWORD_LENGTH             50

namespace hs {

enum DeviceType {
    __t2l_a4l = 0,
    __tiny_1b,
};

enum RequestType {
    __req_init = 0,
    __req_frame,
    __req_shutter,
    __req_frame_mode,
    __req_camera_param,
    __req_hotspot_info,
    __req_infrared,
    __req_disconnect,
    __req_invalid,
};

enum FrameMode {
    __fps_first = 0,        // send half frame data
    __image_first,          // send full frame data
    __invalid_frame_mode,
};

enum FrameFormat {
    __even_frame = 0,
    __odd_frame,
    __full_frame,
    __invalid_frame_format,
};

enum InfraredState
{
    __infrared_off = 0,
    __infrered_on,
};

enum PixelFormat
{
    __yuyv = 0,
    __yuv420,
    __xtherm_n16,
    __gray,
    __invalid_format,
};

typedef struct
{
    // 固定为
    // marker[0] = 'T'
    // marker[1] = 'c'
    // marker[2] = 'A'
    // marker[3] = 'm'
    char marker[4];
    // unsigned char deviceName[32];
    unsigned char devType;

    long long timestamp;

    // see FrameMode
    unsigned char frameMode;
    // see FrameFormat
    // request full frame, frameFormat = FrameFormat::__full_frame
    // request half frame, frameFormat = FrameFormat::__even_frame or FrameFormat::__odd_frame
    unsigned char frameFormat;
    // see PixelFormat
    unsigned char pixelFormat;

    unsigned short width;
    unsigned short height;
    unsigned int bufferLength;

    // pressed key value
    unsigned char key;
    // 0: hotspot disenable
    // 1: hotspot enable
    // app hotspot info is same as the device hotspot info, hotspot enable
    unsigned char hotspotState;

    unsigned char infraredState;

    // version ends with a '\0'
    char version[20];

    bool isValidHeader() {
        return isValidHeader(this->marker);
    }
    bool isValidHeader(const char *m) {
        if( m[0] == 'T'
                && m[1] == 'c'
                && m[2] == 'A'
                && m[3] == 'm' ) {
            return true;
        }
        return false;
    }
} t_header;

typedef struct {
    // 必填
    // marker[0] = 'R'
    // marker[1] = 'e'
    // marker[2] = 'C'
    // marker[3] = 'v'
    char marker[4];
    long long timestamp;

    unsigned char request;

    // see FrameMode
    unsigned char frameMode;
    // see FrameFormat
    // request full frame, frameFormat = FrameFormat::__full_frame
    // request half frame, frameFormat = FrameFormat::__even_frame or FrameFormat::__odd_frame
    unsigned char frameFormat;

    // camera param
    float emiss;
    float reflected;
    float ambient;
    float humidness;
    float correction;
    unsigned short distance;

    // hotspot info
    char hotspotSSID[HOTSPOT_SSID_LENGTH];
    char hotspotPassword[HOTSPOT_PASSWORD_LENGTH];

    unsigned char infraredState;

    bool isValidHeader() {
        return isValidHeader(this->marker);
    }
    bool isValidHeader(const char *m) {
        if( m[0] == 'R'
                && m[1] == 'e'
                && m[2] == 'C'
                && m[3] == 'v' ) {
            return true;
        }
        return false;
    }
} t_packet;

}

#endif
