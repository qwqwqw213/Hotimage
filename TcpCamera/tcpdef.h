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

#define SERVER_IP       "192.168.1.1"
#define SERVER_PORT     27015

typedef struct
{
    QString ip;
    int port;
    t_handshake_page cam;
    t_setting_page set;
} tcp_config;

typedef struct
{
    int r;
    int g;
    int b;
} RGBPixel;

typedef unsigned char YUV;
typedef unsigned char RGB;
typedef unsigned char GRAY;

class convert
{
public:
    int frameSize(const PixelFormat &format, const int &w, const int &h) {
        switch (format) {
        case __yuyv: { return (w * h * 2); }
        case __yuv420: { return (w * h * 3 / 2); }
        default: return 0;
        }
    }

    int yuv_to_rgb_pixel(unsigned char y, unsigned char u, unsigned char v, RGBPixel *rgb) {
        rgb->r = (4767 * (y - 16) + 6537 * (v - 128)) >> 12;
        rgb->g = (4767 * (y - 16) - 1601 * (u - 128) - 3330 * (v - 128)) >> 12;
        rgb->b = (4767 * (y - 16) + 8265 * (u - 128)) >> 12;

        rgb->r = rgb->r > 255 ? 255 : (rgb->r < 0 ? 0 : rgb->r);
        rgb->g = rgb->g > 255 ? 255 : (rgb->g < 0 ? 0 : rgb->g);
        rgb->b = rgb->b > 255 ? 255 : (rgb->b < 0 ? 0 : rgb->b);
        return 1;
    }

    int yuv422_to_rgb(const YUV *raw, RGB *rgb, const int &width, const int &height)
    {
        int in = 0;
        int out = 0;
        YUV y0;
        YUV u;
        YUV y1;
        YUV v;

        RGBPixel pixel;
        for(in = 0; in < width * height * 2; in += 4)
        {
            y0 = raw[in+0];
            u  = raw[in+1];
            y1 = raw[in+2];
            v  = raw[in+3];

            if( yuv_to_rgb_pixel(y0, u, v, &pixel) == 1 )
            {
                rgb[out++] = static_cast<RGB>(pixel.r);
                rgb[out++] = static_cast<RGB>(pixel.g);
                rgb[out++] = static_cast<RGB>(pixel.b);
            }

            if( yuv_to_rgb_pixel(y1, u, v, &pixel) == 1 )
            {
                rgb[out++] = static_cast<RGB>(pixel.r);
                rgb[out++] = static_cast<RGB>(pixel.g);
                rgb[out++] = static_cast<RGB>(pixel.b);
            }
        }

        return 1;
    }

    int yuv420p_to_rgb(unsigned char *yuv, unsigned char *rgb, const int &width, const int &height)
    {
        const unsigned char *y = yuv;
        const unsigned char *u = yuv + (width * height);
        const unsigned char *v = u + (width * height / 4);

        int offset = 0;
        RGBPixel pixel;
        for(int i = 0; i < height; i ++)
        {
            for(int j = 0; j < width; j ++)
            {
                int y_index = i * width + j;
                int uv_index = (i / 4) * width + (j / 2);

                if( yuv_to_rgb_pixel(y[y_index], u[uv_index], v[uv_index], &pixel) == 1 )
                {
                    rgb[offset ++] = static_cast<unsigned char>(pixel.r);
                    rgb[offset ++] = static_cast<unsigned char>(pixel.g);
                    rgb[offset ++] = static_cast<unsigned char>(pixel.b);
                }
            }
        }

        return 1;
    }

};


#endif // TCPDEF_H
