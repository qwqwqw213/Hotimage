#ifndef __HANDSHAKEDEF_H__
#define __HANDSHAKEDEF_H__

#include "string"

namespace hs {
enum RequestType {
    __req_init = 0,
    __req_frame,
    __req_shutter,
    __req_palette,
    __req_camera_param,
    __req_hotspot_info,
    __req_disconnect,
    __req_invalid,
};

enum FrameFormat {
    __even_frame = 0,
    __odd_frame,
    __full_frame,
};

enum PixelFormat
{
    __yuyv = 0,
    __yuv420,
    __xtherm_n16,
    __invalid_format,
};

enum CameraPalette {
    __white_hot,
    __black_hot,
    __iron,
    __hcr,
    __rainbow,
    __irongray,
};

typedef struct
{
    char marker[4];

    unsigned char frameFormat;
    unsigned char pixelFormat;
    unsigned short width;
    unsigned short height;
    unsigned int bufferLength;
    unsigned char palette;

    // pressed key value
    unsigned char key;
    // 0: hotspot disenable
    // 1: hotspot enable
    // app hotspot info is same as the device hotspot info, hotspot enable
    unsigned char hotspot;

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
    char marker[4];

    unsigned char request;

    // 0: send half frame data
    // 1: send full frame data
    unsigned char frameFormat;

    // if camera mode is 0x8005
    // used camera palette, see CameraPalette
    // mode is 0x8004,
    // used custom palette
    unsigned char cameraMode;
    unsigned char palette;

    // camera param
    float emiss;
    float reflected;
    float ambient;
    float humidness;
    float correction;
    unsigned short distance;

    // hotspot info
    char hotspotSSID[255];
    char hotspotPassword[30];

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
