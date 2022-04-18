#ifndef PIXELOPERATIONS_H
#define PIXELOPERATIONS_H

#include "QDebug"
#include "QElapsedTimer"

#include "stdio.h"
#include "vector"
#include "math.h"
#include "stdlib.h"
#include "cstdint"
using namespace std;

#define RELEASE_BUFFER(x) { if (x) {delete [] x; x = NULL;} }

#define RGB_SIZE                    256
#define PSEUDO_COLOR_TABLE_SIZE     7

typedef unsigned char YUV;
typedef unsigned char RGB;
typedef unsigned char GRAY;

#define R_MAP       0
#define G_MAP       1
#define B_MAP       2

typedef unsigned char   U8;
typedef unsigned char   U24;
typedef unsigned int    U32;

enum PixelFlip
{
    __NoFlip = 0,
    __Horizontally,
    __Vertically,
    __Diagonally,
};

enum PixelChannel
{
    __U8  = 1,  // 8 bit, 1 channel, like gray image
    __U24 = 3,  // 8:8:8 bit, 3 channel, like rgb bgr image
    __U32 = 4,  // 8:8:8:8 bit, 4 channel, like argb rgba image
};

enum PseudoColorTable
{
    __InvalidPseudoTable = -1,
    __WhiteHot,
    __BlackHot,
    __BlackRed,
    __PurpleRedYello,
    __BlueGreenRed,
    __RainbowHighBlue,
    __RainbowHighRed,
};

struct Vec3f
{
    double x;
    double y;
    double z;
    Vec3f(double a, double b, double c)
    {
        x = a;
        y = b;
        z = c;
    }
};

typedef struct
{
    PseudoColorTable current;
    vector<Vec3f> **map;
    RGB *r;
    RGB *g;
    RGB *b;
} PseudoColor;

typedef struct _DOT
{
    int x;
    int y;
    bool operator==(const _DOT &dot) {
        return (this->x == dot.x) && (this->y == dot.y);
    }
} Dot;

typedef struct
{
    int x0;
    int y0;
    int x1;
    int y1;
} Path;

typedef struct
{
    int dotCount;
    Dot *dot;
} Line;

typedef struct
{
    int r;
    int g;
    int b;
} RGBPixel;

typedef struct
{
    unsigned char y;
    unsigned char u;
    unsigned char v;
} YUVPixel;


class PixelOperations
{
public:
    PixelOperations() { m_pseudo = nullptr; }
    ~PixelOperations() { freePseudo(); }

    // v: cr
    // u: cb
    static int yuv_to_rgb_pixel(const YUV &Y, const YUV &U, const YUV &V, RGBPixel *rgb) {
        rgb->r = Y + (1.370705 * (V - 128));
        rgb->g = Y - (0.698001 * (V - 128)) - (0.337633 * (U - 128));
        rgb->b = Y + (1.732446 * (U - 128));

        rgb->r = rgb->r > 255 ? 255 : (rgb->r < 0 ? 0 : rgb->r);
        rgb->g = rgb->g > 255 ? 255 : (rgb->g < 0 ? 0 : rgb->g);
        rgb->b = rgb->b > 255 ? 255 : (rgb->b < 0 ? 0 : rgb->b);
        return 1;
    }

    static GRAY rgb_to_gray_pixel(RGBPixel *rgb_pixel)
    {
        return ((rgb_pixel->r * 30 + rgb_pixel->g * 59 + rgb_pixel->b * 11 + 50) / 100);
    }

    static int rgb_to_yuv_pixel(unsigned char r, unsigned char g, unsigned char b, YUVPixel *yuv) {
        yuv->y = (unsigned char)(((66 * r + 129 * g +  25 * b + 128) >> 8) + 16);
        yuv->u = (unsigned char)(((112 * b - 74 * g - 38 * r) >> 8) + 128);
        yuv->v = (unsigned char)(((112 * r - 94 * g - 18 * b) >> 8) + 128);
        return 1;
    }

    void byte_swap(uint8_t *b0, uint8_t *b1, const int &size)
    {
        uint8_t _byte;
        for(int i = 0; i < size; i ++) {
            _byte = b0[i];
            b0[i] = b1[i];
            b1[i] = _byte;
        }
    }

    void horizontally_u32(uint8_t *data, int width, int height)
    {
        int cx = width / 2;
        int offset;
        U32 *u32 = reinterpret_cast<uint32_t *>(data);
        U32 *front = NULL;
        U32 *back = NULL;
        U32 d;
        for(int i = 0; i < height; i ++) {
            offset = width * i;
            front = u32 + offset;
            back = u32 + (offset + width - 1);
            for(int j = 0; j < cx; j ++) {
                d = *front;
                *front = *back;
                *back = d;
                front ++;
                back --;
            }
        }
    }

    void horizontally_u24(uint8_t *data, int width, int height)
    {
        const int bytesize = 3;
        const int linesize = width * bytesize;
        U24 *front = NULL;
        U24 *back = NULL;
        for(int i = 0; i < height; i ++) {
            front = data + (linesize * i);
            back = front + linesize - bytesize;
            for(int j = 0; j < (linesize / 2); j += bytesize) {
                byte_swap(front, back, bytesize);
                front += bytesize;
                back -= bytesize;
            }
        }
    }

    void horizontally_u8(uint8_t *data, int width, int height)
    {
        int cx = width / 2;
        int offset;
        U8 *front = NULL;
        U8 *back = NULL;
        U8 d;
        for(int i = 0; i < height; i ++) {
            offset = width * i;
            front = data + offset;
            back = data + (offset + width - 1);
            for(int j = 0; j < cx; j ++) {
                d = *front;
                *front = *back;
                *back = d;
                front ++;
                back --;
            }
            front += cx + 1;
            back += (cx + width);
        }
    }

    void vertically_u32(uint8_t *data, int width, int height)
    {
        int cy = height / 2;
        U32 *u32 = reinterpret_cast<U32 *>(data);
        U32 *front = u32;
        U32 *back = NULL;
        U32 d;

        for(int i = 0; i < cy; i ++) {
            back = u32 + (width * (height - i - 1));
            for(int j = 0; j < width; j ++) {
                d = *front;
                *front = *back;
                *back = d;
                front ++;
                back ++;
            }
        }
    }

    void vertically_u24(uint8_t *data, int width, int height)
    {
        const int channel = 3;
        const int offset = channel * width;
        int cy = height / 2;
        U24 *front = data;
        U24 *back = NULL;
        U24 d;

        for(int i = 0; i < cy; i ++) {
            back = data + (offset * (height - i - 1));
            for(int j = 0; j < offset; j ++) {
                d = *front;
                *front = *back;
                *back = d;
                front ++;
                back ++;
            }
        }
    }

    void vertically_u8(uint8_t *data, int width, int height)
    {
        int cy = height / 2;
        U8 *front = data;
        U8 *back = NULL;
        U8 d;

        for(int i = 0; i < cy; i ++) {
            back = data + (width * (height - i - 1));
            for(int j = 0; j < width; j ++) {
                d = *front;
                *front = *back;
                *back = d;
                front ++;
                back ++;
            }
        }
    }

    void diagonally_u32(uint8_t *data, int width, int height)
    {
        U32 *u32 = reinterpret_cast<U32 *>(data);
        U32 *front = u32;
        U32 *back = u32 + (width * height - 1);
        U32 d;

        int count = width * height / 2;
        for(int i = 0; i < count; i ++) {
            d = *front;
            *front = *back;
            *back = d;
            front ++;
            back --;
        }
    }

    void diagonally_u24(uint8_t *data, int width, int height)
    {
        const int bytesize = 3;
        U24 *front = data;
        U24 *back = data + (width * height * bytesize - bytesize);
        int count = width * height / 2;
        for(int i = 0; i < count; i ++) {
            byte_swap(front, back, bytesize);
            front += bytesize;
            back -= bytesize;
        }
    }

    void diagonally_u8(uint8_t *data, int width, int height)
    {
        U8 *front = data;
        U8 *back = data + (width * height - 1);
        U8 d;

        int count = width * height / 2;
        for(int i = 0; i < count; i ++) {
            d = *front;
            *front = *back;
            *back = d;
            front ++;
            back --;
        }
    }

    static int yuv420p_to_rgb(const YUV *raw, RGB *rgb, const int &width, const int &height)
    {
        //
        // yuv420p数据排序
        // 数据总大小为 w * h * 1.5
        // 前 w * h 个数据为 y data
        // 后 w * h * 0.5 个为数据 u, v data
        // u addr = y addr + width * height
        // v addr = u addr + width * height / 4
        //
        //        width
        // +-----------------+
        // |                 |
        // |                 |
        // |     y data      | height
        // |                 |
        // |                 |
        // |                 |
        // +--------+--------+
        // |        |
        // | u data | height / 2
        // |        |
        // +--------+
        // |        |
        // | v data |
        // |        |
        // +--------+
        //  width / 2
        //

        if( (raw == nullptr) || (rgb == nullptr) ) {
            return 0;
        }

        const unsigned char *y = raw;
        const unsigned char *u = raw + (width * height);
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

    static int yuv422_to_rgb(const YUV *raw, RGB *rgb, const int &width, const int &height)
    {
        /*
         *  YUV422数据排序
         *  数据总大小为 w * h * 2
         *  y u v穿插排序
         *  Y0 U0 Y1 V0 .... Y(n) U(n) Y(n + 1) V(n)
         *  4个byte 为 2个RGB24像素, Y0 和 Y1 公用同一个U V数据
         */
        if( (raw == nullptr) || (rgb == nullptr) ) {
            return 0;
        }

        int in, out = 0;
        unsigned char y0, u, y1, v;
        RGBPixel pixel;
        for(in = 0; in < width * height * 2; in += 4)
        {
            y0 = raw[in + 0];
            u  = raw[in + 1];
            y1 = raw[in + 2];
            v  = raw[in + 3];

            yuv_to_rgb_pixel(y0, u, v, &pixel);
            rgb[out++] = pixel.r;
            rgb[out++] = pixel.g;
            rgb[out++] = pixel.b;

            yuv_to_rgb_pixel(y1, u, v, &pixel);
            rgb[out++] = pixel.r;
            rgb[out++] = pixel.g;
            rgb[out++] = pixel.b;
        }
        return 1;
    }

    static int yuv422p_to_rgb(const YUV *raw, RGB *rgb, const int &width, const int &height)
    {
        /*
         *  yuv422p 像素排序
         *  yuv422p 数据大小为 width * height * 2
         *  前 width * height 个数据为 y data
         *  后 width * height 个数据为 u, v data
         *
         *           width
         *  +---------------------+
         *  |                     |
         *  |                     |
         *  |       y data        |  height
         *  |                     |
         *  |                     |
         *  |                     |
         *  +---------------------+
         *  |                     |
         *  |       u data        |  height / 2
         *  |                     |
         *  +---------------------+
         *  |                     |
         *  |       v data        |
         *  |                     |
         *  +---------------------+
         *
         */
        if( (raw == nullptr) || (rgb == nullptr) ) {
            return 0;
        }

        const YUV *y_addr = raw;
        const YUV *u_addr = y_addr + width * height;
        const YUV *v_addr = y_addr + (width * height * 3 / 2);

        YUV y0;
        YUV u;
        YUV y1;
        YUV v;

        int y_size = width * height * 2;

        RGBPixel pixel;
        for(int i = 0; i < y_size; i += 4)
        {
            y0 = *(y_addr ++);
            u  = *(u_addr ++);
            y1 = *(y_addr ++);
            v  = *(v_addr ++);

            if( yuv_to_rgb_pixel(y0, u, v, &pixel) == 1 )
            {
                *(rgb ++) = static_cast<RGB>(pixel.r);
                *(rgb ++) = static_cast<RGB>(pixel.b);
                *(rgb ++) = static_cast<RGB>(pixel.g);
            }

            if( yuv_to_rgb_pixel(y1, u, v, &pixel) == 1 )
            {
                *(rgb ++) = static_cast<RGB>(pixel.r);
                *(rgb ++) = static_cast<RGB>(pixel.b);
                *(rgb ++) = static_cast<RGB>(pixel.g);
            }
        }

        return 1;
    }

    void flip(uint8_t *data, int width, int height, PixelFlip state, PixelChannel channel)
    {
        switch (state) {
        case PixelFlip::__Horizontally: {
            if( channel == PixelChannel::__U8 ) { horizontally_u8(data, width, height); }
            else if( channel == PixelChannel::__U24 ) { horizontally_u24(data, width, height); }
            else if( channel == PixelChannel::__U32 ) { horizontally_u32(data, width, height); }
        }
            break;
        case PixelFlip::__Vertically: {
            if( channel == PixelChannel::__U8 ) { vertically_u8(data, width, height); }
            else if( channel == PixelChannel::__U24 ) { vertically_u24(data, width, height); }
            else if( channel == PixelChannel::__U32 ) { vertically_u32(data, width, height); }
        }
            break;
        case PixelFlip::__Diagonally: {
            if( channel == PixelChannel::__U8 ) { diagonally_u8(data, width, height); }
            else if( channel == PixelChannel::__U24 ) { diagonally_u24(data, width, height); }
            else if( channel == PixelChannel::__U32 ) { diagonally_u32(data, width, height); }
        }
            break;
        default: break;
        }
    }

    bool hasPseudoTable() { return (m_pseudo != nullptr); }

    int pseudoCount() {
        return PSEUDO_COLOR_TABLE_SIZE;
    }

    bool initializerPseudo(const PseudoColorTable &table)
    {
        if( hasPseudoTable() ) {
            return true;
        }
        PseudoColor *pseudo = new PseudoColor;
        pseudo->map = new vector<Vec3f>*[PSEUDO_COLOR_TABLE_SIZE];
        for(int i = 0; i < PSEUDO_COLOR_TABLE_SIZE; i ++) {
            pseudo->map[i] = new vector<Vec3f>[3];
        }
        pseudo->r = new RGB[RGB_SIZE];
        pseudo->g = new RGB[RGB_SIZE];
        pseudo->b = new RGB[RGB_SIZE];

        {
           vector<Vec3f> r;
           r.push_back(Vec3f(0.0, 0.0, 0.0));
           r.push_back(Vec3f(0.111111111111, 0.222222222222, 0.222222222222));
           r.push_back(Vec3f(0.222222222222, 0.444444444444, 0.444444444444));
           r.push_back(Vec3f(0.333333333333, 0.666666666667, 0.666666666667));
           r.push_back(Vec3f(0.444444444444, 0.888888888889, 0.888888888889));
           r.push_back(Vec3f(0.555555555556, 1.0, 1.0));
           r.push_back(Vec3f(0.666666666667, 1.0, 1.0));
           r.push_back(Vec3f(0.777777777778, 1.0, 1.0));
           r.push_back(Vec3f(0.888888888889, 1.0, 1.0));
           r.push_back(Vec3f(1.0, 1.0, 1.0));
           vector<Vec3f> g;
           g.push_back(Vec3f(0.0, 0.0, 0.0));
           g.push_back(Vec3f(0.111111111111, 0.0, 0.0));
           g.push_back(Vec3f(0.222222222222, 0.0, 0.0));
           g.push_back(Vec3f(0.333333333333, 0.166666666667, 0.166666666667));
           g.push_back(Vec3f(0.444444444444, 0.388888888889, 0.388888888889));
           g.push_back(Vec3f(0.555555555556, 0.611111111111, 0.611111111111));
           g.push_back(Vec3f(0.666666666667, 0.833333333333, 0.833333333333));
           g.push_back(Vec3f(0.777777777778, 1.0, 1.0));
           g.push_back(Vec3f(0.888888888889, 1.0, 1.0));
           g.push_back(Vec3f(1.0, 1.0, 1.0));
           vector<Vec3f> b;
           b.push_back(Vec3f(0.0, 0.0, 0.0));
           b.push_back(Vec3f(0.111111111111, 0.0, 0.0));
           b.push_back(Vec3f(0.222222222222, 0.0, 0.0));
           b.push_back(Vec3f(0.333333333333, 0.0, 0.0));
           b.push_back(Vec3f(0.444444444444, 0.0, 0.0));
           b.push_back(Vec3f(0.555555555556, 0.111111111111, 0.111111111111));
           b.push_back(Vec3f(0.666666666667, 0.333333333333, 0.333333333333));
           b.push_back(Vec3f(0.777777777778, 0.555555555556, 0.555555555556));
           b.push_back(Vec3f(0.888888888889, 0.777777777778, 0.777777777778));
           b.push_back(Vec3f(1.0, 1.0, 1.0));
           pseudo->map[__BlackRed][R_MAP] = r; pseudo->map[__BlackRed][G_MAP] = g; pseudo->map[__BlackRed][B_MAP] = b;
       }

       {
           vector<Vec3f> r;
           r.push_back(Vec3f(0.0, 1.0, 1.0));
           r.push_back(Vec3f(0.111111111111, 0.888888888889, 0.888888888889));
           r.push_back(Vec3f(0.222222222222, 0.777777777778, 0.777777777778));
           r.push_back(Vec3f(0.333333333333, 0.666666666667, 0.666666666667));
           r.push_back(Vec3f(0.444444444444, 0.555555555556, 0.555555555556));
           r.push_back(Vec3f(0.555555555556, 0.444444444444, 0.444444444444));
           r.push_back(Vec3f(0.666666666667, 0.333333333333, 0.333333333333));
           r.push_back(Vec3f(0.777777777778, 0.222222222222, 0.222222222222));
           r.push_back(Vec3f(0.888888888889, 0.111111111111, 0.111111111111));
           r.push_back(Vec3f(1.0, 0.0, 0.0));
           vector<Vec3f> g;
           g.push_back(Vec3f(0.0, 1.0, 1.0));
           g.push_back(Vec3f(0.111111111111, 0.888888888889, 0.888888888889));
           g.push_back(Vec3f(0.222222222222, 0.777777777778, 0.777777777778));
           g.push_back(Vec3f(0.333333333333, 0.666666666667, 0.666666666667));
           g.push_back(Vec3f(0.444444444444, 0.555555555556, 0.555555555556));
           g.push_back(Vec3f(0.555555555556, 0.444444444444, 0.444444444444));
           g.push_back(Vec3f(0.666666666667, 0.333333333333, 0.333333333333));
           g.push_back(Vec3f(0.777777777778, 0.222222222222, 0.222222222222));
           g.push_back(Vec3f(0.888888888889, 0.111111111111, 0.111111111111));
           g.push_back(Vec3f(1.0, 0.0, 0.0));
           vector<Vec3f> b;
           b.push_back(Vec3f(0.0, 1.0, 1.0));
           b.push_back(Vec3f(0.111111111111, 0.888888888889, 0.888888888889));
           b.push_back(Vec3f(0.222222222222, 0.777777777778, 0.777777777778));
           b.push_back(Vec3f(0.333333333333, 0.666666666667, 0.666666666667));
           b.push_back(Vec3f(0.444444444444, 0.555555555556, 0.555555555556));
           b.push_back(Vec3f(0.555555555556, 0.444444444444, 0.444444444444));
           b.push_back(Vec3f(0.666666666667, 0.333333333333, 0.333333333333));
           b.push_back(Vec3f(0.777777777778, 0.222222222222, 0.222222222222));
           b.push_back(Vec3f(0.888888888889, 0.111111111111, 0.111111111111));
           b.push_back(Vec3f(1.0, 0.0, 0.0));
           pseudo->map[__BlackHot][R_MAP] = r; pseudo->map[__BlackHot][G_MAP] = g; pseudo->map[__BlackHot][B_MAP] = b;
        }

        {
           vector<Vec3f> r;
           r.push_back(Vec3f(0.0, 0.0, 0.0));
           r.push_back(Vec3f(0.111111111111, 0.111111111111, 0.111111111111));
           r.push_back(Vec3f(0.222222222222, 0.222222222222, 0.222222222222));
           r.push_back(Vec3f(0.333333333333, 0.333333333333, 0.333333333333));
           r.push_back(Vec3f(0.444444444444, 0.444444444444, 0.444444444444));
           r.push_back(Vec3f(0.555555555556, 0.555555555556, 0.555555555556));
           r.push_back(Vec3f(0.666666666667, 0.666666666667, 0.666666666667));
           r.push_back(Vec3f(0.777777777778, 0.777777777778, 0.777777777778));
           r.push_back(Vec3f(0.888888888889, 0.888888888889, 0.888888888889));
           r.push_back(Vec3f(1.0, 1.0, 1.0));
           vector<Vec3f> g;
           g.push_back(Vec3f(0.0, 0.0, 0.0));
           g.push_back(Vec3f(0.111111111111, 0.111111111111, 0.111111111111));
           g.push_back(Vec3f(0.222222222222, 0.222222222222, 0.222222222222));
           g.push_back(Vec3f(0.333333333333, 0.333333333333, 0.333333333333));
           g.push_back(Vec3f(0.444444444444, 0.444444444444, 0.444444444444));
           g.push_back(Vec3f(0.555555555556, 0.555555555556, 0.555555555556));
           g.push_back(Vec3f(0.666666666667, 0.666666666667, 0.666666666667));
           g.push_back(Vec3f(0.777777777778, 0.777777777778, 0.777777777778));
           g.push_back(Vec3f(0.888888888889, 0.888888888889, 0.888888888889));
           g.push_back(Vec3f(1.0, 1.0, 1.0));
           vector<Vec3f> b;
           b.push_back(Vec3f(0.0, 0.0, 0.0));
           b.push_back(Vec3f(0.111111111111, 0.111111111111, 0.111111111111));
           b.push_back(Vec3f(0.222222222222, 0.222222222222, 0.222222222222));
           b.push_back(Vec3f(0.333333333333, 0.333333333333, 0.333333333333));
           b.push_back(Vec3f(0.444444444444, 0.444444444444, 0.444444444444));
           b.push_back(Vec3f(0.555555555556, 0.555555555556, 0.555555555556));
           b.push_back(Vec3f(0.666666666667, 0.666666666667, 0.666666666667));
           b.push_back(Vec3f(0.777777777778, 0.777777777778, 0.777777777778));
           b.push_back(Vec3f(0.888888888889, 0.888888888889, 0.888888888889));
           b.push_back(Vec3f(1.0, 1.0, 1.0));
           pseudo->map[__WhiteHot][R_MAP] = r; pseudo->map[__WhiteHot][G_MAP] = g; pseudo->map[__WhiteHot][B_MAP] = b;
        }

        {
           vector<Vec3f> r;
           r.push_back(Vec3f(0.0,1.0,1.0));
           r.push_back(Vec3f(0.03,1.0,1.0));
           r.push_back(Vec3f(0.215,1.0,1.0));
           r.push_back(Vec3f(0.4,0.0,0.0));
           r.push_back(Vec3f(0.586,0.0,0.0));
           r.push_back(Vec3f(0.77,0.0,0.0));
           r.push_back(Vec3f(0.954,1.0,1.0));
           r.push_back(Vec3f(1.0,1.0,1.0));
           vector<Vec3f> g;
           g.push_back(Vec3f(0.0,0.0,0.0));
           g.push_back(Vec3f(0.03,0.0,0.0));
           g.push_back(Vec3f(0.215,1.0,1.0));
           g.push_back(Vec3f(0.4,1.0,1.0));
           g.push_back(Vec3f(0.586,1.0,1.0));
           g.push_back(Vec3f(0.77,0.0,0.0));
           g.push_back(Vec3f(0.954,0.0,0.0));
           g.push_back(Vec3f(1.0,0.0,0.0));
           vector<Vec3f> b;
           b.push_back(Vec3f(0.0,0.16,0.16));
           b.push_back(Vec3f(0.03,0.0,0.0));
           b.push_back(Vec3f(0.215,0.0,0.0));
           b.push_back(Vec3f(0.4,0.0,0.0));
           b.push_back(Vec3f(0.586,1.0,1.0));
           b.push_back(Vec3f(0.77,1.0,1.0));
           b.push_back(Vec3f(0.954,1.0,1.0));
           b.push_back(Vec3f(1.0,0.75,0.75));
           pseudo->map[__RainbowHighBlue][R_MAP] = r; pseudo->map[__RainbowHighBlue][G_MAP] = g; pseudo->map[__RainbowHighBlue][B_MAP] = b;
        }
        {
           vector<Vec3f> r;
           r.push_back(Vec3f(0.0,0.5,0.5));
           r.push_back(Vec3f(0.111111111111,0.277777777778,0.277777777778));
           r.push_back(Vec3f(0.222222222222,0.0555555555556,0.0555555555556));
           r.push_back(Vec3f(0.333333333333,0.166666666667,0.166666666667));
           r.push_back(Vec3f(0.444444444444,0.388888888889,0.388888888889));
           r.push_back(Vec3f(0.555555555556,0.611111111111,0.611111111111));
           r.push_back(Vec3f(0.666666666667,0.833333333333,0.833333333333));
           r.push_back(Vec3f(0.777777777778,1.0,1.0));
           r.push_back(Vec3f(0.888888888889,1.0,1.0));
           r.push_back(Vec3f(1.0,1.0,1.0));
           vector<Vec3f> g;
           g.push_back(Vec3f(0.0,0.0,0.0));
           g.push_back(Vec3f(0.111111111111,0.342020143326,0.342020143326));
           g.push_back(Vec3f(0.222222222222,0.642787609687,0.642787609687));
           g.push_back(Vec3f(0.333333333333,0.866025403784,0.866025403784));
           g.push_back(Vec3f(0.444444444444,0.984807753012,0.984807753012));
           g.push_back(Vec3f(0.555555555556,0.984807753012,0.984807753012));
           g.push_back(Vec3f(0.666666666667,0.866025403784,0.866025403784));
           g.push_back(Vec3f(0.777777777778,0.642787609687,0.642787609687));
           g.push_back(Vec3f(0.888888888889,0.342020143326,0.342020143326));
           g.push_back(Vec3f(1.0,1.22464679915e-16,1.22464679915e-16));
           vector<Vec3f> b;
           b.push_back(Vec3f(0.0,1.0,1.0));
           b.push_back(Vec3f(0.111111111111,0.984807753012,0.984807753012));
           b.push_back(Vec3f(0.222222222222,0.939692620786,0.939692620786));
           b.push_back(Vec3f(0.333333333333,0.866025403784,0.866025403784));
           b.push_back(Vec3f(0.444444444444,0.766044443119,0.766044443119));
           b.push_back(Vec3f(0.555555555556,0.642787609687,0.642787609687));
           b.push_back(Vec3f(0.666666666667,0.5,0.5));
           b.push_back(Vec3f(0.777777777778,0.342020143326,0.342020143326));
           b.push_back(Vec3f(0.888888888889,0.173648177667,0.173648177667));
           b.push_back(Vec3f(1.0,6.12323399574e-17,6.12323399574e-17));
           pseudo->map[__RainbowHighRed][R_MAP] = r; pseudo->map[__RainbowHighRed][G_MAP] = g; pseudo->map[__RainbowHighRed][B_MAP] = b;
        }
        {
           vector<Vec3f> r;
           r.push_back(Vec3f(0.0, 0.0,0.0));
           r.push_back(Vec3f(0.051, 0.10, 0.10));
           r.push_back(Vec3f(0.125, 0.39, 0.39));
           r.push_back(Vec3f(0.250, 0.48, 0.48));
           r.push_back(Vec3f(0.375, 0.60, 0.60));
           r.push_back(Vec3f(0.501, 0.90, 0.90));
           r.push_back(Vec3f(0.625, 0.90, 0.90));
           r.push_back(Vec3f(0.750, 0.90, 0.90));
           r.push_back(Vec3f(0.875, 0.90, 0.90));
           r.push_back(Vec3f(1.0,0.9,0.9));
           vector<Vec3f> g;
           g.push_back(Vec3f(0.0, 0.0,0.0));
           g.push_back(Vec3f(0.111, 0.01, 0.01));
           g.push_back(Vec3f(0.125, 0.01, 0.01));
           g.push_back(Vec3f(0.250, 0.01, 0.01));
           g.push_back(Vec3f(0.375, 0.01, 0.01));
           g.push_back(Vec3f(0.501, 0.25, 0.25));
           g.push_back(Vec3f(0.625, 0.50, 0.50));
           g.push_back(Vec3f(0.750, 0.75, 0.75));
           g.push_back(Vec3f(0.875, 0.90, 0.90));
           g.push_back(Vec3f(1.0,0.9,0.9));
           vector<Vec3f> b;
           b.push_back(Vec3f(0.0,0.0,0.0));
           b.push_back(Vec3f(0.051, 0.32, 0.32));
           b.push_back(Vec3f(0.125, 0.35, 0.35));
           b.push_back(Vec3f(0.250, 0.44, 0.44));
           b.push_back(Vec3f(0.375, 0.55, 0.55));
           b.push_back(Vec3f(0.501, 0.25, 0.25));
           b.push_back(Vec3f(0.625, 0.01, 0.01));
           b.push_back(Vec3f(0.750, 0.30, 0.30));
           b.push_back(Vec3f(0.875, 0.50, 0.50));
           b.push_back(Vec3f(1.0,0.7,0.7));
           pseudo->map[__PurpleRedYello][R_MAP] = r; pseudo->map[__PurpleRedYello][G_MAP] = g; pseudo->map[__PurpleRedYello][B_MAP] = b;
        }
        {
           vector<Vec3f> r;
           r.push_back(Vec3f(0.0, 0.0, 0.0));
           r.push_back(Vec3f(0.15, 0.16, 0.16));
           r.push_back(Vec3f(0.35, 0.11, 0.11));
           r.push_back(Vec3f(0.49, 0.11, 0.11));
           r.push_back(Vec3f(0.61, 0.15, 0.15));
           r.push_back(Vec3f(0.74, 0.61, 0.61));
           r.push_back(Vec3f(0.81, 0.75, 0.75));
           r.push_back(Vec3f(1.0, 0.8, 0.8));
           vector<Vec3f> g;
           g.push_back(Vec3f(0.0, 0.0, 0.0));
           g.push_back(Vec3f(0.15, 0.34, 0.34));
           g.push_back(Vec3f(0.35, 0.15, 0.15));
           g.push_back(Vec3f(0.49, 0.61, 0.61));
           g.push_back(Vec3f(0.61, 0.75, 0.75));
           g.push_back(Vec3f(0.74, 0.12, 0.12));
           g.push_back(Vec3f(0.81, 0.05, 0.05));
           g.push_back(Vec3f(1.0, 0.05, 0.05));
           vector<Vec3f> b;
           b.push_back(Vec3f(0.0, 0.0, 0.0));
           b.push_back(Vec3f(0.15, 0.85, 0.85));
           b.push_back(Vec3f(0.35, 0.68, 0.68));
           b.push_back(Vec3f(0.49, 0.21, 0.21));
           b.push_back(Vec3f(0.61, 0.33, 0.33));
           b.push_back(Vec3f(0.74, 0.01, 0.01));
           b.push_back(Vec3f(0.81, 0.01, 0.01));
           b.push_back(Vec3f(1.0, 0.05, 0.05));
           pseudo->map[__BlueGreenRed][R_MAP] = r; pseudo->map[__BlueGreenRed][G_MAP] = g; pseudo->map[__BlueGreenRed][B_MAP] = b;
        }
        m_pseudo = pseudo;

        updatePseudoColor(table);
        return true;
    }

    PseudoColorTable currentPseudoTable() {
        if( !hasPseudoTable() ) {
            return __InvalidPseudoTable;
        }
        return m_pseudo->current;
    }

    void updatePseudoColor(const PseudoColorTable &table)
    {
        if( !hasPseudoTable() ) {
            return;
        }
        m_pseudo->current = table;
        vector<Vec3f> *colorMap = m_pseudo->map[table];

        channelTable(m_pseudo->r, colorMap[R_MAP]);
        channelTable(m_pseudo->g, colorMap[G_MAP]);
        channelTable(m_pseudo->b, colorMap[B_MAP]);
    }

    void channelTable(unsigned char *table, vector<Vec3f> &data)
    {

        int idx = 0;
        for(int i = 0; i < RGB_SIZE; i ++)
        {
            float r = i / 255.0f;
            if( (r < 1.0) && (r >= data[idx + 1].x) ) {
                idx ++;
            }
            double prop = (r - data[idx].x) / (data[idx + 1].x - data[idx].x);
            double val = data[idx].z + (data[idx + 1].y - data[idx].z) * prop;
            table[i] = (int)(255 * val);
        }
    }

    void gray_to_pseudo(RGB *rgb, GRAY *gray, int width, int height)
    {
        if( !hasPseudoTable() ) {
            return;
        }
        GRAY _gray;
        for(int i = 0; i < height; i ++) {
            for(int j = 0; j < width; j ++) {
                _gray = gray[width * i + j];
                *rgb ++ = m_pseudo->r[_gray];
                *rgb ++ = m_pseudo->g[_gray];
                *rgb ++ = m_pseudo->b[_gray];
            }
        }
    }

    void yuv422_to_pseudo(YUV *yuyv, RGB *rgb, int width, int height)
    {
        if( !hasPseudoTable() ) {
            return;
        }

    //    RGBPixel rgb_pixel;
        int in = 0;
        YUV y0;
        YUV u;
        YUV y1;
        YUV v;
    //    GRAY gray;
        for(in = 0; in < width * height * 2; in += 4)
        {
            y0 = yuyv[in + 0];
            u  = yuyv[in + 1];
            y1 = yuyv[in + 2];
            v  = yuyv[in + 3];

    //        if( yuv_to_rgb_pixel(y0, u, v, &rgb_pixel) == 1 )
    //        {
    //            gray = rgb_to_gray_pixel(&rgb_pixel);
                *rgb ++ = m_pseudo->r[y0];
                *rgb ++ = m_pseudo->g[y0];
                *rgb ++ = m_pseudo->b[y0];
    //        }

    //        if( yuv_to_rgb_pixel(y1, u, v, &rgb_pixel) == 1 )
    //        {
    //            gray = rgb_to_gray_pixel(&rgb_pixel);
                *rgb ++ = m_pseudo->r[y1];
                *rgb ++ = m_pseudo->g[y1];
                *rgb ++ = m_pseudo->b[y1];
    //        }
        }
    }

    void gray_to_rgb_pixel(const GRAY &gray, RGBPixel *rgb)
    {
        if( !hasPseudoTable() ) {
            return;
        }
        rgb->r = m_pseudo->r[gray];
        rgb->g = m_pseudo->g[gray];
        rgb->b = m_pseudo->b[gray];
    }

    void freePseudo()
    {
        if( !hasPseudoTable() ) {
            return;
        }

        for(int i = 0; i < PSEUDO_COLOR_TABLE_SIZE; i ++) {
            delete [] m_pseudo->map[i];
            m_pseudo->map[i] = NULL;
        }

        delete [] m_pseudo->map;
        m_pseudo->map = NULL;

        delete [] m_pseudo->r;
        m_pseudo->r = NULL;

        delete [] m_pseudo->g;
        m_pseudo->g = NULL;

        delete [] m_pseudo->b;
        m_pseudo->b = NULL;

        delete m_pseudo;
        m_pseudo = NULL;
    }

    int lineDot(const Path &path, Line *line)
    {
        int x0 = path.x0;
        int x1 = path.x1;
        int y0 = path.y0;
        int y1 = path.y1;
        Dot dot;

        int dx = abs(x0 - x1);
        int dy = abs(y0 - y1);
        if( (dx < 1) || (dy < 1) ) {
            return 0;
        }

        int size = (int)sqrt(pow(dx, 2) + pow(dy, 2));
        if( !line->dot && (size > 0) ) {
            line->dot = new Dot[size];
        }
        else {
            return 0;
        }

        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = (dx > dy ?  dx : -dy) / 2;

        line->dotCount = 0;
        while ((x0 != x1) || (y0 != y1))
        {
            int e = err;
            if( e > (-dx) )
            {
                err -= dy;
                x0 += sx;
            }
            if( e < dy )
            {
                err += dx;
                y0 += sy;
            }

            dot.x = x0;
            dot.y = y0;
            line->dot[line->dotCount] = dot;
            line->dotCount ++;
        }
        return 1;
    }

    void freeLine(Line &line)
    {
        if( line.dot ) {
            delete [] line.dot;
            line.dot = NULL;

            line.dotCount = 0;
        }
    }

private:
    PseudoColor *m_pseudo;
};

#endif
