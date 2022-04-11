#ifndef __HANDSHAKEDEF_H__
#define __HANDSHAKEDEF_H__

typedef struct
{
    char marker[4];
    unsigned int bufferLength;
    unsigned short width;
    unsigned short height;
    unsigned char pixelFormat;
    // 0: off, 1: on
    // bit 1: key
    // bit 2: hotspot mode
    unsigned char value;

    bool isValidHeader() {
        if( this->marker[0] == 'T'
                && this->marker[1] == 'c'
                && this->marker[2] == 'A'
                && this->marker[3] == 'm' ) {
            return true;
        }
        return false;
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

enum PixelFormat
{
    __yuyv = 0,
    __yuv420,
};

#endif
