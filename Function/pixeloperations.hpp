#ifndef PIXELOPERATIONS_H
#define PIXELOPERATIONS_H

#include "QDebug"
#include "QElapsedTimer"

#include "iostream"
#include "stdio.h"
#include "vector"
#include "math.h"
#include "stdlib.h"
#include "cstdint"
#include "memory"
using namespace std;

#define RELEASE_BUFFER(x) { if (x) {delete [] x; x = NULL;} }

#define RGB_SIZE                    256

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
    __Pseudo_WhiteHot = 0,
    __Pseudo_BlackHot,
    __Pseudo_Iron,
    __Pseudo_HCR,
    __Pseudo_Rainbow,
    __Pseudo_IronGray,
    __Pseudo_Ocean,
    __Pseudo_Summer,
    __Pseudo_Hot,
    __InvalidPseudoTable,
};

enum ContrastLevel {
    __no_contrast = 0,
    __contrast_1,
    __contrast_2,
    __contrast_3,
    __contrast_4,
    __contrast_5,
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

//typedef struct
//{
//    PseudoColorTable current;
//    vector<Vec3f> **map;
//    RGB *r;
//    RGB *g;
//    RGB *b;
//} PseudoColor;

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
    PixelOperations() { }
    PixelOperations(const PseudoColorTable &table) { initializerPseudo(table); }
    ~PixelOperations() { }

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

    static int yuv420p_to_rgb(const YUV *in, RGB *out, const int &width, const int &height)
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

        if( (in == nullptr) || (out == nullptr) ) {
            return 0;
        }

        const unsigned char *y = in;
        const unsigned char *u = in + (width * height);
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
                    out[offset ++] = static_cast<unsigned char>(pixel.r);
                    out[offset ++] = static_cast<unsigned char>(pixel.g);
                    out[offset ++] = static_cast<unsigned char>(pixel.b);
                }
            }
        }

        return 1;
    }

    static int yuv422_to_rgb(const YUV *in, RGB *out, const int &width, const int &height)
    {
        /*
         *  YUV422数据排序
         *  数据总大小为 w * h * 2
         *  y u v穿插排序
         *  Y0 U0 Y1 V0 .... Y(n) U(n) Y(n + 1) V(n)
         *  4个byte 为 2个RGB24像素, Y0 和 Y1 公用同一个U V数据
         */
        if( (in == nullptr) || (out == nullptr) ) {
            return 0;
        }

        unsigned char y0, u, y1, v;
        RGBPixel pixel;
        for(int i = 0; i < width * height * 2; i += 4) {
            y0 = in[i + 0];
            u  = in[i + 1];
            y1 = in[i + 2];
            v  = in[i + 3];

            yuv_to_rgb_pixel(y0, u, v, &pixel);
            *out ++ = pixel.r;
            *out ++ = pixel.g;
            *out ++ = pixel.b;

            yuv_to_rgb_pixel(y1, u, v, &pixel);
            *out ++ = pixel.r;
            *out ++ = pixel.g;
            *out ++ = pixel.b;
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

    bool initializerPseudo(const PseudoColorTable &table)
    {
        pseudoColor.reset(new PseudoColor);
        pseudoColor->updatePseudoColorTable(table);
        return true;
    }

    PseudoColorTable currentPseudoColorTable() {
        return pseudoColor->currentPseudoColorTable();
    }

    void updatePseudoColor(const PseudoColorTable &table)
    {
        pseudoColor->updatePseudoColorTable(table);
    }

    void gray_to_pseudo(GRAY *in, RGB *out, const int &width, const int &height) {
        pseudoColor->gray_to_pseudo(in, out, width, height);
    }

    void yuv420p_to_pseudo(YUV *in, RGB *out, const int &width, const int &height) {
        pseudoColor->yuv420p_to_pseudo(in, out, width, height);
    }

    void yuv422_to_pseudo(YUV *in, RGB *out, const int &width, const int &height) {
        pseudoColor->yuv422_to_pseudo(in, out, width, height);
    }

    void gray_to_rgb_pixel(const GRAY &gray, RGBPixel *rgb) {
        pseudoColor->gray_to_rgb_pixel(gray, rgb);
    }

    ContrastLevel contrastLevel() { return pseudoColor->contrastLevel(); }
    void setContrastLevel(const ContrastLevel &contrast) {
        pseudoColor->setContrastLevel(contrast);
    }

    const vector<Vec3f> &pseudoColorMap(const int &index) {
        return pseudoColor->pseudoColorMap(index);
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
    class PseudoColor {
    public:
        PseudoColor() {
            r_ptr.reset(new RGB[RGB_SIZE]);
            g_ptr.reset(new RGB[RGB_SIZE]);
            b_ptr.reset(new RGB[RGB_SIZE]);
            current_table = __InvalidPseudoTable;
            contrast = __no_contrast;
        }
        ~PseudoColor() { }

        PseudoColorTable currentPseudoColorTable() { return current_table; }
        void updatePseudoColorTable(const PseudoColorTable &table) {
            if( table < __Pseudo_WhiteHot
                    || table >= __InvalidPseudoTable ) {
                printf("ERROR: pseudo index wrong\n");
                return;
            }

            current_table = table;

            vector<Vec3f> r = vector<Vec3f>();
            vector<Vec3f> g = vector<Vec3f>();
            vector<Vec3f> b = vector<Vec3f>();
            switch (table) {
            case __Pseudo_WhiteHot: {
                r.push_back(Vec3f(0.00, 0.01, 0.01));
                r.push_back(Vec3f(0.05, 0.05, 0.05));
                r.push_back(Vec3f(0.10, 0.11, 0.11));
                r.push_back(Vec3f(0.15, 0.15, 0.15));
                r.push_back(Vec3f(0.20, 0.21, 0.21));
                r.push_back(Vec3f(0.25, 0.25, 0.25));
                r.push_back(Vec3f(0.30, 0.31, 0.31));
                r.push_back(Vec3f(0.35, 0.35, 0.35));
                r.push_back(Vec3f(0.40, 0.41, 0.41));
                r.push_back(Vec3f(0.45, 0.45, 0.45));
                r.push_back(Vec3f(0.50, 0.51, 0.51));
                r.push_back(Vec3f(0.55, 0.55, 0.55));
                r.push_back(Vec3f(0.60, 0.61, 0.61));
                r.push_back(Vec3f(0.65, 0.65, 0.65));
                r.push_back(Vec3f(0.70, 0.71, 0.71));
                r.push_back(Vec3f(0.75, 0.75, 0.75));
                r.push_back(Vec3f(0.80, 0.81, 0.81));
                r.push_back(Vec3f(0.85, 0.85, 0.85));
                r.push_back(Vec3f(0.90, 0.96, 0.96));
                r.push_back(Vec3f(0.95, 0.99, 0.99));
                r.push_back(Vec3f(1.00, 1.00, 1.00));

                g.push_back(Vec3f(0.00, 0.01, 0.01));
                g.push_back(Vec3f(0.05, 0.05, 0.05));
                g.push_back(Vec3f(0.10, 0.11, 0.11));
                g.push_back(Vec3f(0.15, 0.15, 0.15));
                g.push_back(Vec3f(0.20, 0.21, 0.21));
                g.push_back(Vec3f(0.25, 0.25, 0.25));
                g.push_back(Vec3f(0.30, 0.31, 0.31));
                g.push_back(Vec3f(0.35, 0.35, 0.35));
                g.push_back(Vec3f(0.40, 0.41, 0.41));
                g.push_back(Vec3f(0.45, 0.45, 0.45));
                g.push_back(Vec3f(0.50, 0.51, 0.51));
                g.push_back(Vec3f(0.55, 0.55, 0.55));
                g.push_back(Vec3f(0.60, 0.61, 0.61));
                g.push_back(Vec3f(0.65, 0.65, 0.65));
                g.push_back(Vec3f(0.70, 0.71, 0.71));
                g.push_back(Vec3f(0.75, 0.75, 0.75));
                g.push_back(Vec3f(0.80, 0.81, 0.81));
                g.push_back(Vec3f(0.85, 0.85, 0.85));
                g.push_back(Vec3f(0.90, 0.96, 0.96));
                g.push_back(Vec3f(0.95, 0.99, 0.99));
                g.push_back(Vec3f(1.00, 1.00, 1.00));

                b.push_back(Vec3f(0.00, 0.01, 0.01));
                b.push_back(Vec3f(0.05, 0.05, 0.05));
                b.push_back(Vec3f(0.10, 0.11, 0.11));
                b.push_back(Vec3f(0.15, 0.15, 0.15));
                b.push_back(Vec3f(0.20, 0.21, 0.21));
                b.push_back(Vec3f(0.25, 0.25, 0.25));
                b.push_back(Vec3f(0.30, 0.31, 0.31));
                b.push_back(Vec3f(0.35, 0.35, 0.35));
                b.push_back(Vec3f(0.40, 0.41, 0.41));
                b.push_back(Vec3f(0.45, 0.45, 0.45));
                b.push_back(Vec3f(0.50, 0.51, 0.51));
                b.push_back(Vec3f(0.55, 0.55, 0.55));
                b.push_back(Vec3f(0.60, 0.61, 0.61));
                b.push_back(Vec3f(0.65, 0.65, 0.65));
                b.push_back(Vec3f(0.70, 0.71, 0.71));
                b.push_back(Vec3f(0.75, 0.75, 0.75));
                b.push_back(Vec3f(0.80, 0.81, 0.81));
                b.push_back(Vec3f(0.85, 0.85, 0.85));
                b.push_back(Vec3f(0.90, 0.96, 0.96));
                b.push_back(Vec3f(0.95, 0.99, 0.99));
                b.push_back(Vec3f(1.00, 1.00, 1.00));
            }
                break;
            case __Pseudo_BlackHot: {
                r.push_back(Vec3f(0.00, 1.00, 1.00));
                r.push_back(Vec3f(0.05, 0.99, 0.99));
                r.push_back(Vec3f(0.10, 0.96, 0.96));
                r.push_back(Vec3f(0.15, 0.85, 0.85));
                r.push_back(Vec3f(0.20, 0.81, 0.81));
                r.push_back(Vec3f(0.25, 0.75, 0.75));
                r.push_back(Vec3f(0.30, 0.71, 0.71));
                r.push_back(Vec3f(0.35, 0.65, 0.65));
                r.push_back(Vec3f(0.40, 0.61, 0.61));
                r.push_back(Vec3f(0.45, 0.55, 0.55));
                r.push_back(Vec3f(0.50, 0.51, 0.51));
                r.push_back(Vec3f(0.55, 0.45, 0.45));
                r.push_back(Vec3f(0.60, 0.41, 0.41));
                r.push_back(Vec3f(0.65, 0.35, 0.35));
                r.push_back(Vec3f(0.70, 0.31, 0.31));
                r.push_back(Vec3f(0.75, 0.25, 0.25));
                r.push_back(Vec3f(0.80, 0.21, 0.21));
                r.push_back(Vec3f(0.85, 0.15, 0.15));
                r.push_back(Vec3f(0.90, 0.11, 0.11));
                r.push_back(Vec3f(0.95, 0.05, 0.05));
                r.push_back(Vec3f(1.00, 0.01, 0.01));

                g.push_back(Vec3f(0.00, 1.00, 1.00));
                g.push_back(Vec3f(0.05, 0.99, 0.99));
                g.push_back(Vec3f(0.10, 0.96, 0.96));
                g.push_back(Vec3f(0.15, 0.85, 0.85));
                g.push_back(Vec3f(0.20, 0.81, 0.81));
                g.push_back(Vec3f(0.25, 0.75, 0.75));
                g.push_back(Vec3f(0.30, 0.71, 0.71));
                g.push_back(Vec3f(0.35, 0.65, 0.65));
                g.push_back(Vec3f(0.40, 0.61, 0.61));
                g.push_back(Vec3f(0.45, 0.55, 0.55));
                g.push_back(Vec3f(0.50, 0.51, 0.51));
                g.push_back(Vec3f(0.55, 0.45, 0.45));
                g.push_back(Vec3f(0.60, 0.41, 0.41));
                g.push_back(Vec3f(0.65, 0.35, 0.35));
                g.push_back(Vec3f(0.70, 0.31, 0.31));
                g.push_back(Vec3f(0.75, 0.25, 0.25));
                g.push_back(Vec3f(0.80, 0.21, 0.21));
                g.push_back(Vec3f(0.85, 0.15, 0.15));
                g.push_back(Vec3f(0.90, 0.11, 0.11));
                g.push_back(Vec3f(0.95, 0.05, 0.05));
                g.push_back(Vec3f(1.00, 0.01, 0.01));

                b.push_back(Vec3f(0.00, 1.00, 1.00));
                b.push_back(Vec3f(0.05, 0.99, 0.99));
                b.push_back(Vec3f(0.10, 0.96, 0.96));
                b.push_back(Vec3f(0.15, 0.85, 0.85));
                b.push_back(Vec3f(0.20, 0.81, 0.81));
                b.push_back(Vec3f(0.25, 0.75, 0.75));
                b.push_back(Vec3f(0.30, 0.71, 0.71));
                b.push_back(Vec3f(0.35, 0.65, 0.65));
                b.push_back(Vec3f(0.40, 0.61, 0.61));
                b.push_back(Vec3f(0.45, 0.55, 0.55));
                b.push_back(Vec3f(0.50, 0.51, 0.51));
                b.push_back(Vec3f(0.55, 0.45, 0.45));
                b.push_back(Vec3f(0.60, 0.41, 0.41));
                b.push_back(Vec3f(0.65, 0.35, 0.35));
                b.push_back(Vec3f(0.70, 0.31, 0.31));
                b.push_back(Vec3f(0.75, 0.25, 0.25));
                b.push_back(Vec3f(0.80, 0.21, 0.21));
                b.push_back(Vec3f(0.85, 0.15, 0.15));
                b.push_back(Vec3f(0.90, 0.11, 0.11));
                b.push_back(Vec3f(0.95, 0.05, 0.05));
                b.push_back(Vec3f(1.00, 0.01, 0.01));
            }
                break;
            case __Pseudo_Iron: {
                r.push_back(Vec3f(0.00, 0.01, 0.01));
                r.push_back(Vec3f(0.05, 0.11, 0.11));
                r.push_back(Vec3f(0.10, 0.21, 0.21));
                r.push_back(Vec3f(0.15, 0.31, 0.31));
                r.push_back(Vec3f(0.20, 0.41, 0.41));
                r.push_back(Vec3f(0.25, 0.51, 0.51));
                r.push_back(Vec3f(0.30, 0.61, 0.61));
                r.push_back(Vec3f(0.35, 0.71, 0.71));
                r.push_back(Vec3f(0.40, 0.75, 0.75));
                r.push_back(Vec3f(0.45, 0.81, 0.81));
                r.push_back(Vec3f(0.50, 0.85, 0.85));
                r.push_back(Vec3f(0.55, 0.87, 0.87));
                r.push_back(Vec3f(0.60, 0.89, 0.89));
                r.push_back(Vec3f(0.65, 0.91, 0.91));
                r.push_back(Vec3f(0.70, 0.98, 0.98));
                r.push_back(Vec3f(0.75, 0.99, 0.99));
                r.push_back(Vec3f(0.80, 0.99, 0.99));
                r.push_back(Vec3f(0.85, 0.99, 0.99));
                r.push_back(Vec3f(0.90, 0.99, 0.99));
                r.push_back(Vec3f(0.95, 0.99, 0.99));
                r.push_back(Vec3f(1.00, 1, 1));

                g.push_back(Vec3f(0.00, 0.01, 0.01));
                g.push_back(Vec3f(0.05, 0.01, 0.01));
                g.push_back(Vec3f(0.10, 0.01, 0.01));
                g.push_back(Vec3f(0.15, 0.01, 0.01));
                g.push_back(Vec3f(0.20, 0.01, 0.01));
                g.push_back(Vec3f(0.25, 0.01, 0.01));
                g.push_back(Vec3f(0.30, 0.01, 0.01));
                g.push_back(Vec3f(0.35, 0.11, 0.11));
                g.push_back(Vec3f(0.40, 0.15, 0.15));
                g.push_back(Vec3f(0.45, 0.25, 0.25));
                g.push_back(Vec3f(0.50, 0.35, 0.35));
                g.push_back(Vec3f(0.55, 0.41, 0.41));
                g.push_back(Vec3f(0.60, 0.45, 0.45));
                g.push_back(Vec3f(0.65, 0.55, 0.55));
                g.push_back(Vec3f(0.70, 0.61, 0.61));
                g.push_back(Vec3f(0.75, 0.65, 0.65));
                g.push_back(Vec3f(0.80, 0.71, 0.71));
                g.push_back(Vec3f(0.85, 0.75, 0.75));
                g.push_back(Vec3f(0.90, 0.81, 0.81));
                g.push_back(Vec3f(0.95, 0.85, 0.85));
                g.push_back(Vec3f(1.00, 1, 1));

                b.push_back(Vec3f(0.00, 0.15, 0.15));
                b.push_back(Vec3f(0.05, 0.31, 0.31));
                b.push_back(Vec3f(0.10, 0.41, 0.41));
                b.push_back(Vec3f(0.15, 0.51, 0.51));
                b.push_back(Vec3f(0.20, 0.61, 0.61));
                b.push_back(Vec3f(0.25, 0.71, 0.71));
                b.push_back(Vec3f(0.30, 0.81, 0.81));
                b.push_back(Vec3f(0.35, 0.55, 0.55));
                b.push_back(Vec3f(0.40, 0.35, 0.35));
                b.push_back(Vec3f(0.45, 0.21, 0.21));
                b.push_back(Vec3f(0.50, 0.11, 0.11));
                b.push_back(Vec3f(0.55, 0.05, 0.05));
                b.push_back(Vec3f(0.60, 0.01, 0.01));
                b.push_back(Vec3f(0.65, 0.01, 0.01));
                b.push_back(Vec3f(0.70, 0.01, 0.01));
                b.push_back(Vec3f(0.75, 0.05, 0.05));
                b.push_back(Vec3f(0.80, 0.11, 0.11));
                b.push_back(Vec3f(0.85, 0.15, 0.15));
                b.push_back(Vec3f(0.90, 0.35, 0.35));
                b.push_back(Vec3f(0.95, 0.55, 0.55));
                b.push_back(Vec3f(1.00, 1, 1));
            }
                break;
            case __Pseudo_IronGray: {
                r.push_back(Vec3f(0.00, 0.99, 0.99));
                r.push_back(Vec3f(0.05, 0.99, 0.99));
                r.push_back(Vec3f(0.10, 0.95, 0.95));
                r.push_back(Vec3f(0.15, 0.91, 0.91));
                r.push_back(Vec3f(0.20, 0.81, 0.81));
                r.push_back(Vec3f(0.25, 0.71, 0.71));
                r.push_back(Vec3f(0.30, 0.61, 0.61));
                r.push_back(Vec3f(0.35, 0.55, 0.55));
                r.push_back(Vec3f(0.40, 0.48, 0.48));
                r.push_back(Vec3f(0.45, 0.41, 0.41));
                r.push_back(Vec3f(0.50, 0.41, 0.41));
                r.push_back(Vec3f(0.55, 0.35, 0.35));
                r.push_back(Vec3f(0.60, 0.29, 0.29));
                r.push_back(Vec3f(0.65, 0.32, 0.32));
                r.push_back(Vec3f(0.70, 0.34, 0.34));
                r.push_back(Vec3f(0.75, 0.36, 0.36));
                r.push_back(Vec3f(0.80, 0.38, 0.38));
                r.push_back(Vec3f(0.85, 0.42, 0.42));
                r.push_back(Vec3f(0.90, 0.45, 0.45));
                r.push_back(Vec3f(0.95, 0.48, 0.48));
                r.push_back(Vec3f(1.00, 0.51, 0.51));

                g.push_back(Vec3f(0.00, 0.99, 0.99));
                g.push_back(Vec3f(0.05, 0.95, 0.95));
                g.push_back(Vec3f(0.10, 0.91, 0.91));
                g.push_back(Vec3f(0.15, 0.85, 0.85));
                g.push_back(Vec3f(0.20, 0.75, 0.75));
                g.push_back(Vec3f(0.25, 0.65, 0.65));
                g.push_back(Vec3f(0.30, 0.51, 0.51));
                g.push_back(Vec3f(0.35, 0.45, 0.45));
                g.push_back(Vec3f(0.40, 0.41, 0.41));
                g.push_back(Vec3f(0.45, 0.38, 0.38));
                g.push_back(Vec3f(0.50, 0.31, 0.31));
                g.push_back(Vec3f(0.55, 0.25, 0.25));
                g.push_back(Vec3f(0.60, 0.21, 0.21));
                g.push_back(Vec3f(0.65, 0.15, 0.15));
                g.push_back(Vec3f(0.70, 0.11, 0.11));
                g.push_back(Vec3f(0.75, 0.11, 0.11));
                g.push_back(Vec3f(0.80, 0.11, 0.11));
                g.push_back(Vec3f(0.85, 0.11, 0.11));
                g.push_back(Vec3f(0.90, 0.01, 0.01));
                g.push_back(Vec3f(0.95, 0.01, 0.01));
                g.push_back(Vec3f(1.00, 0.01, 0.01));

                b.push_back(Vec3f(0.00, 0.01, 0.01));
                b.push_back(Vec3f(0.05, 0.05, 0.05));
                b.push_back(Vec3f(0.10, 0.11, 0.11));
                b.push_back(Vec3f(0.15, 0.15, 0.15));
                b.push_back(Vec3f(0.20, 0.17, 0.17));
                b.push_back(Vec3f(0.25, 0.19, 0.19));
                b.push_back(Vec3f(0.30, 0.21, 0.21));
                b.push_back(Vec3f(0.35, 0.25, 0.25));
                b.push_back(Vec3f(0.40, 0.31, 0.31));
                b.push_back(Vec3f(0.45, 0.35, 0.35));
                b.push_back(Vec3f(0.50, 0.45, 0.45));
                b.push_back(Vec3f(0.55, 0.47, 0.47));
                b.push_back(Vec3f(0.60, 0.41, 0.41));
                b.push_back(Vec3f(0.65, 0.39, 0.39));
                b.push_back(Vec3f(0.70, 0.41, 0.41));
                b.push_back(Vec3f(0.75, 0.43, 0.43));
                b.push_back(Vec3f(0.80, 0.46, 0.46));
                b.push_back(Vec3f(0.85, 0.48, 0.48));
                b.push_back(Vec3f(0.90, 0.51, 0.51));
                b.push_back(Vec3f(0.95, 0.53, 0.53));
                b.push_back(Vec3f(1.00, 0.55, 0.55));
            }
                break;
            case __Pseudo_Rainbow: {
                r.push_back(Vec3f(0.00, 0.01, 0.01));
                r.push_back(Vec3f(0.05, 0.03, 0.03));
                r.push_back(Vec3f(0.10, 0.05, 0.05));
                r.push_back(Vec3f(0.15, 0.11, 0.11));
                r.push_back(Vec3f(0.20, 0.13, 0.13));
                r.push_back(Vec3f(0.25, 0.15, 0.15));
                r.push_back(Vec3f(0.30, 0.19, 0.19));
                r.push_back(Vec3f(0.35, 0.24, 0.24));
                r.push_back(Vec3f(0.40, 0.38, 0.38));
                r.push_back(Vec3f(0.45, 0.54, 0.54));
                r.push_back(Vec3f(0.50, 0.78, 0.78));
                r.push_back(Vec3f(0.55, 0.83, 0.83));
                r.push_back(Vec3f(0.60, 0.85, 0.85));
                r.push_back(Vec3f(0.65, 0.87, 0.87));
                r.push_back(Vec3f(0.70, 0.89, 0.89));
                r.push_back(Vec3f(0.75, 0.91, 0.91));
                r.push_back(Vec3f(0.80, 0.93, 0.93));
                r.push_back(Vec3f(0.85, 0.95, 0.95));
                r.push_back(Vec3f(0.90, 0.97, 0.97));
                r.push_back(Vec3f(0.95, 0.99, 0.99));
                r.push_back(Vec3f(1.00, 1, 1));

                g.push_back(Vec3f(0.00, 0.01, 0.01));
                g.push_back(Vec3f(0.05, 0.05, 0.05));
                g.push_back(Vec3f(0.10, 0.15, 0.15));
                g.push_back(Vec3f(0.15, 0.25, 0.25));
                g.push_back(Vec3f(0.20, 0.35, 0.35));
                g.push_back(Vec3f(0.25, 0.45, 0.45));
                g.push_back(Vec3f(0.30, 0.65, 0.65));
                g.push_back(Vec3f(0.35, 0.69, 0.69));
                g.push_back(Vec3f(0.40, 0.75, 0.75));
                g.push_back(Vec3f(0.45, 0.83, 0.83));
                g.push_back(Vec3f(0.50, 0.91, 0.91));
                g.push_back(Vec3f(0.55, 0.83, 0.83));
                g.push_back(Vec3f(0.60, 0.75, 0.75));
                g.push_back(Vec3f(0.65, 0.52, 0.52));
                g.push_back(Vec3f(0.70, 0.39, 0.39));
                g.push_back(Vec3f(0.75, 0.15, 0.15));
                g.push_back(Vec3f(0.80, 0.24, 0.24));
                g.push_back(Vec3f(0.85, 0.48, 0.48));
                g.push_back(Vec3f(0.90, 0.65, 0.65));
                g.push_back(Vec3f(0.95, 0.81, 0.81));
                g.push_back(Vec3f(1.00, 1, 1));

                b.push_back(Vec3f(0.00, 0.11, 0.11));
                b.push_back(Vec3f(0.05, 0.18, 0.18));
                b.push_back(Vec3f(0.10, 0.36, 0.36));
                b.push_back(Vec3f(0.15, 0.52, 0.52));
                b.push_back(Vec3f(0.20, 0.76, 0.76));
                b.push_back(Vec3f(0.25, 0.99, 0.99));
                b.push_back(Vec3f(0.30, 0.76, 0.76));
                b.push_back(Vec3f(0.35, 0.52, 0.52));
                b.push_back(Vec3f(0.40, 0.36, 0.36));
                b.push_back(Vec3f(0.45, 0.18, 0.18));
                b.push_back(Vec3f(0.50, 0.11, 0.11));
                b.push_back(Vec3f(0.55, 0.05, 0.05));
                b.push_back(Vec3f(0.60, 0.01, 0.01));
                b.push_back(Vec3f(0.65, 0.05, 0.05));
                b.push_back(Vec3f(0.70, 0.19, 0.19));
                b.push_back(Vec3f(0.75, 0.27, 0.27));
                b.push_back(Vec3f(0.80, 0.39, 0.39));
                b.push_back(Vec3f(0.85, 0.51, 0.51));
                b.push_back(Vec3f(0.90, 0.69, 0.69));
                b.push_back(Vec3f(0.95, 0.83, 0.83));
                b.push_back(Vec3f(1.00, 1, 1));

            }
                break;
            case __Pseudo_HCR: {
                r.push_back(Vec3f(0.00, 0.01, 0.01));
                r.push_back(Vec3f(0.05, 0.01, 0.01));
                r.push_back(Vec3f(0.10, 0.01, 0.01));
                r.push_back(Vec3f(0.15, 0.01, 0.01));
                r.push_back(Vec3f(0.20, 0.15, 0.15));
                r.push_back(Vec3f(0.25, 0.21, 0.21));
                r.push_back(Vec3f(0.30, 0.26, 0.26));
                r.push_back(Vec3f(0.35, 0.35, 0.35));
                r.push_back(Vec3f(0.40, 0.45, 0.45));
                r.push_back(Vec3f(0.45, 0.59, 0.59));
                r.push_back(Vec3f(0.50, 0.71, 0.71));
                r.push_back(Vec3f(0.55, 0.76, 0.76));
                r.push_back(Vec3f(0.60, 0.82, 0.82));
                r.push_back(Vec3f(0.65, 0.89, 0.89));
                r.push_back(Vec3f(0.70, 0.91, 0.91));
                r.push_back(Vec3f(0.75, 0.99, 0.99));
                r.push_back(Vec3f(0.80, 0.99, 0.99));
                r.push_back(Vec3f(0.85, 0.99, 0.99));
                r.push_back(Vec3f(0.90, 0.99, 0.99));
                r.push_back(Vec3f(0.95, 0.99, 0.99));
                r.push_back(Vec3f(1.00, 1.00, 1.00));

                g.push_back(Vec3f(0.00, 0.01, 0.01));
                g.push_back(Vec3f(0.05, 0.11, 0.11));
                g.push_back(Vec3f(0.10, 0.21, 0.21));
                g.push_back(Vec3f(0.15, 0.35, 0.35));
                g.push_back(Vec3f(0.20, 0.45, 0.45));
                g.push_back(Vec3f(0.25, 0.55, 0.55));
                g.push_back(Vec3f(0.30, 0.61, 0.61));
                g.push_back(Vec3f(0.35, 0.11, 0.11));
                g.push_back(Vec3f(0.40, 0.11, 0.11));
                g.push_back(Vec3f(0.45, 0.11, 0.11));
                g.push_back(Vec3f(0.50, 0.11, 0.11));
                g.push_back(Vec3f(0.55, 0.21, 0.21));
                g.push_back(Vec3f(0.60, 0.31, 0.31));
                g.push_back(Vec3f(0.65, 0.45, 0.45));
                g.push_back(Vec3f(0.70, 0.55, 0.55));
                g.push_back(Vec3f(0.75, 0.61, 0.61));
                g.push_back(Vec3f(0.80, 0.69, 0.69));
                g.push_back(Vec3f(0.85, 0.71, 0.71));
                g.push_back(Vec3f(0.90, 0.79, 0.79));
                g.push_back(Vec3f(0.95, 0.85, 0.85));
                g.push_back(Vec3f(1.00, 1.00, 1.00));

                b.push_back(Vec3f(0.00, 0.11, 0.11));
                b.push_back(Vec3f(0.05, 0.35, 0.35));
                b.push_back(Vec3f(0.10, 0.45, 0.45));
                b.push_back(Vec3f(0.15, 0.48, 0.48));
                b.push_back(Vec3f(0.20, 0.55, 0.55));
                b.push_back(Vec3f(0.25, 0.58, 0.58));
                b.push_back(Vec3f(0.30, 0.61, 0.61));
                b.push_back(Vec3f(0.35, 0.51, 0.51));
                b.push_back(Vec3f(0.40, 0.41, 0.41));
                b.push_back(Vec3f(0.45, 0.36, 0.36));
                b.push_back(Vec3f(0.50, 0.21, 0.21));
                b.push_back(Vec3f(0.55, 0.11, 0.11));
                b.push_back(Vec3f(0.60, 0.15, 0.15));
                b.push_back(Vec3f(0.65, 0.19, 0.19));
                b.push_back(Vec3f(0.70, 0.24, 0.24));
                b.push_back(Vec3f(0.75, 0.21, 0.21));
                b.push_back(Vec3f(0.80, 0.26, 0.26));
                b.push_back(Vec3f(0.85, 0.31, 0.31));
                b.push_back(Vec3f(0.90, 0.35, 0.35));
                b.push_back(Vec3f(0.95, 0.45, 0.45));
                b.push_back(Vec3f(1.00, 1.00, 1.00));
            }
                break;
            case __Pseudo_Ocean: {
                r.push_back(Vec3f(0.00, 0.01, 0.01));
                r.push_back(Vec3f(0.05, 0.01, 0.01));
                r.push_back(Vec3f(0.10, 0.01, 0.01));
                r.push_back(Vec3f(0.15, 0.01, 0.01));
                r.push_back(Vec3f(0.20, 0.01, 0.01));
                r.push_back(Vec3f(0.25, 0.01, 0.01));
                r.push_back(Vec3f(0.30, 0.01, 0.01));
                r.push_back(Vec3f(0.35, 0.11, 0.11));
                r.push_back(Vec3f(0.40, 0.15, 0.15));
                r.push_back(Vec3f(0.45, 0.21, 0.21));
                r.push_back(Vec3f(0.50, 0.25, 0.25));
                r.push_back(Vec3f(0.55, 0.31, 0.31));
                r.push_back(Vec3f(0.60, 0.35, 0.35));
                r.push_back(Vec3f(0.65, 0.41, 0.41));
                r.push_back(Vec3f(0.70, 0.45, 0.45));
                r.push_back(Vec3f(0.75, 0.53, 0.53));
                r.push_back(Vec3f(0.80, 0.64, 0.64));
                r.push_back(Vec3f(0.85, 0.75, 0.75));
                r.push_back(Vec3f(0.90, 0.81, 0.81));
                r.push_back(Vec3f(0.95, 0.85, 0.85));
                r.push_back(Vec3f(1.00, 1, 1));

                g.push_back(Vec3f(0.00, 0.01, 0.01));
                g.push_back(Vec3f(0.05, 0.11, 0.11));
                g.push_back(Vec3f(0.10, 0.15, 0.15));
                g.push_back(Vec3f(0.15, 0.21, 0.21));
                g.push_back(Vec3f(0.20, 0.25, 0.25));
                g.push_back(Vec3f(0.25, 0.31, 0.31));
                g.push_back(Vec3f(0.30, 0.35, 0.35));
                g.push_back(Vec3f(0.35, 0.41, 0.41));
                g.push_back(Vec3f(0.40, 0.45, 0.45));
                g.push_back(Vec3f(0.45, 0.51, 0.51));
                g.push_back(Vec3f(0.50, 0.55, 0.55));
                g.push_back(Vec3f(0.55, 0.59, 0.59));
                g.push_back(Vec3f(0.60, 0.62, 0.62));
                g.push_back(Vec3f(0.65, 0.68, 0.68));
                g.push_back(Vec3f(0.70, 0.71, 0.71));
                g.push_back(Vec3f(0.75, 0.79, 0.79));
                g.push_back(Vec3f(0.80, 0.85, 0.85));
                g.push_back(Vec3f(0.85, 0.91, 0.91));
                g.push_back(Vec3f(0.90, 0.94, 0.94));
                g.push_back(Vec3f(0.95, 0.99, 0.99));
                g.push_back(Vec3f(1.00, 1, 1));

                b.push_back(Vec3f(0.00, 0.11, 0.11));
                b.push_back(Vec3f(0.05, 0.15, 0.15));
                b.push_back(Vec3f(0.10, 0.21, 0.21));
                b.push_back(Vec3f(0.15, 0.31, 0.31));
                b.push_back(Vec3f(0.20, 0.37, 0.37));
                b.push_back(Vec3f(0.25, 0.46, 0.46));
                b.push_back(Vec3f(0.30, 0.53, 0.53));
                b.push_back(Vec3f(0.35, 0.56, 0.56));
                b.push_back(Vec3f(0.40, 0.59, 0.59));
                b.push_back(Vec3f(0.45, 0.62, 0.62));
                b.push_back(Vec3f(0.50, 0.66, 0.66));
                b.push_back(Vec3f(0.55, 0.69, 0.69));
                b.push_back(Vec3f(0.60, 0.72, 0.72));
                b.push_back(Vec3f(0.65, 0.75, 0.75));
                b.push_back(Vec3f(0.70, 0.79, 0.79));
                b.push_back(Vec3f(0.75, 0.85, 0.85));
                b.push_back(Vec3f(0.80, 0.91, 0.91));
                b.push_back(Vec3f(0.85, 0.96, 0.96));
                b.push_back(Vec3f(0.90, 0.97, 0.97));
                b.push_back(Vec3f(0.95, 0.99, 0.99));
                b.push_back(Vec3f(1.00, 1, 1));
            }
                break;
            case __Pseudo_Summer: {
                r.push_back(Vec3f(0.00, 0.11, 0.11));
                r.push_back(Vec3f(0.05, 0.13, 0.13));
                r.push_back(Vec3f(0.10, 0.17, 0.17));
                r.push_back(Vec3f(0.15, 0.18, 0.18));
                r.push_back(Vec3f(0.20, 0.19, 0.19));
                r.push_back(Vec3f(0.25, 0.21, 0.21));
                r.push_back(Vec3f(0.30, 0.23, 0.23));
                r.push_back(Vec3f(0.35, 0.25, 0.25));
                r.push_back(Vec3f(0.40, 0.31, 0.31));
                r.push_back(Vec3f(0.45, 0.33, 0.33));
                r.push_back(Vec3f(0.50, 0.35, 0.35));
                r.push_back(Vec3f(0.55, 0.45, 0.45));
                r.push_back(Vec3f(0.60, 0.51, 0.51));
                r.push_back(Vec3f(0.65, 0.55, 0.55));
                r.push_back(Vec3f(0.70, 0.61, 0.61));
                r.push_back(Vec3f(0.75, 0.65, 0.65));
                r.push_back(Vec3f(0.80, 0.71, 0.71));
                r.push_back(Vec3f(0.85, 0.75, 0.75));
                r.push_back(Vec3f(0.90, 0.81, 0.81));
                r.push_back(Vec3f(0.95, 0.85, 0.85));
                r.push_back(Vec3f(1.00, 0.99, 0.99));

                g.push_back(Vec3f(0.00, 0.21, 0.21));
                g.push_back(Vec3f(0.05, 0.25, 0.25));
                g.push_back(Vec3f(0.10, 0.29, 0.29));
                g.push_back(Vec3f(0.15, 0.32, 0.32));
                g.push_back(Vec3f(0.20, 0.35, 0.35));
                g.push_back(Vec3f(0.25, 0.39, 0.39));
                g.push_back(Vec3f(0.30, 0.42, 0.42));
                g.push_back(Vec3f(0.35, 0.46, 0.46));
                g.push_back(Vec3f(0.40, 0.49, 0.49));
                g.push_back(Vec3f(0.45, 0.51, 0.51));
                g.push_back(Vec3f(0.50, 0.55, 0.55));
                g.push_back(Vec3f(0.55, 0.58, 0.58));
                g.push_back(Vec3f(0.60, 0.61, 0.61));
                g.push_back(Vec3f(0.65, 0.65, 0.65));
                g.push_back(Vec3f(0.70, 0.71, 0.71));
                g.push_back(Vec3f(0.75, 0.75, 0.75));
                g.push_back(Vec3f(0.80, 0.81, 0.81));
                g.push_back(Vec3f(0.85, 0.85, 0.85));
                g.push_back(Vec3f(0.90, 0.91, 0.91));
                g.push_back(Vec3f(0.95, 0.95, 0.95));
                g.push_back(Vec3f(1.00, 0.99, 0.99));

                b.push_back(Vec3f(0.00, 0.11, 0.11));
                b.push_back(Vec3f(0.05, 0.17, 0.17));
                b.push_back(Vec3f(0.10, 0.23, 0.23));
                b.push_back(Vec3f(0.15, 0.25, 0.25));
                b.push_back(Vec3f(0.20, 0.27, 0.27));
                b.push_back(Vec3f(0.25, 0.29, 0.29));
                b.push_back(Vec3f(0.30, 0.31, 0.31));
                b.push_back(Vec3f(0.35, 0.33, 0.33));
                b.push_back(Vec3f(0.40, 0.35, 0.35));
                b.push_back(Vec3f(0.45, 0.37, 0.37));
                b.push_back(Vec3f(0.50, 0.38, 0.38));
                b.push_back(Vec3f(0.55, 0.39, 0.39));
                b.push_back(Vec3f(0.60, 0.41, 0.41));
                b.push_back(Vec3f(0.65, 0.42, 0.42));
                b.push_back(Vec3f(0.70, 0.43, 0.43));
                b.push_back(Vec3f(0.75, 0.44, 0.44));
                b.push_back(Vec3f(0.80, 0.45, 0.45));
                b.push_back(Vec3f(0.85, 0.46, 0.46));
                b.push_back(Vec3f(0.90, 0.47, 0.47));
                b.push_back(Vec3f(0.95, 0.48, 0.48));
                b.push_back(Vec3f(1.00, 0.51, 0.51));
            }
                break;
            case __Pseudo_Hot: {
                r.push_back(Vec3f(0.00, 0.11, 0.11));
                r.push_back(Vec3f(0.05, 0.19, 0.19));
                r.push_back(Vec3f(0.10, 0.25, 0.25));
                r.push_back(Vec3f(0.15, 0.29, 0.29));
                r.push_back(Vec3f(0.20, 0.35, 0.35));
                r.push_back(Vec3f(0.25, 0.41, 0.41));
                r.push_back(Vec3f(0.30, 0.45, 0.45));
                r.push_back(Vec3f(0.35, 0.51, 0.51));
                r.push_back(Vec3f(0.40, 0.55, 0.55));
                r.push_back(Vec3f(0.45, 0.61, 0.61));
                r.push_back(Vec3f(0.50, 0.65, 0.65));
                r.push_back(Vec3f(0.55, 0.68, 0.68));
                r.push_back(Vec3f(0.60, 0.75, 0.75));
                r.push_back(Vec3f(0.65, 0.77, 0.77));
                r.push_back(Vec3f(0.70, 0.79, 0.79));
                r.push_back(Vec3f(0.75, 0.81, 0.81));
                r.push_back(Vec3f(0.80, 0.85, 0.85));
                r.push_back(Vec3f(0.85, 0.91, 0.91));
                r.push_back(Vec3f(0.90, 0.95, 0.95));
                r.push_back(Vec3f(0.95, 0.99, 0.99));
                r.push_back(Vec3f(1.00, 1, 1));

                g.push_back(Vec3f(0.00, 0.01, 0.01));
                g.push_back(Vec3f(0.05, 0.03, 0.03));
                g.push_back(Vec3f(0.10, 0.05, 0.05));
                g.push_back(Vec3f(0.15, 0.07, 0.07));
                g.push_back(Vec3f(0.20, 0.09, 0.09));
                g.push_back(Vec3f(0.25, 0.11, 0.11));
                g.push_back(Vec3f(0.30, 0.13, 0.13));
                g.push_back(Vec3f(0.35, 0.15, 0.15));
                g.push_back(Vec3f(0.40, 0.17, 0.17));
                g.push_back(Vec3f(0.45, 0.19, 0.19));
                g.push_back(Vec3f(0.50, 0.21, 0.21));
                g.push_back(Vec3f(0.55, 0.23, 0.23));
                g.push_back(Vec3f(0.60, 0.27, 0.27));
                g.push_back(Vec3f(0.65, 0.36, 0.36));
                g.push_back(Vec3f(0.70, 0.45, 0.45));
                g.push_back(Vec3f(0.75, 0.53, 0.53));
                g.push_back(Vec3f(0.80, 0.62, 0.62));
                g.push_back(Vec3f(0.85, 0.71, 0.71));
                g.push_back(Vec3f(0.90, 0.82, 0.82));
                g.push_back(Vec3f(0.95, 0.91, 0.91));
                g.push_back(Vec3f(1.00, 1, 1));

                b.push_back(Vec3f(0.00, 0.01, 0.01));
                b.push_back(Vec3f(0.05, 0.02, 0.02));
                b.push_back(Vec3f(0.10, 0.03, 0.03));
                b.push_back(Vec3f(0.15, 0.04, 0.04));
                b.push_back(Vec3f(0.20, 0.05, 0.05));
                b.push_back(Vec3f(0.25, 0.06, 0.06));
                b.push_back(Vec3f(0.30, 0.07, 0.07));
                b.push_back(Vec3f(0.35, 0.08, 0.08));
                b.push_back(Vec3f(0.40, 0.09, 0.09));
                b.push_back(Vec3f(0.45, 0.11, 0.11));
                b.push_back(Vec3f(0.50, 0.13, 0.13));
                b.push_back(Vec3f(0.55, 0.15, 0.15));
                b.push_back(Vec3f(0.60, 0.17, 0.17));
                b.push_back(Vec3f(0.65, 0.19, 0.19));
                b.push_back(Vec3f(0.70, 0.21, 0.21));
                b.push_back(Vec3f(0.75, 0.23, 0.23));
                b.push_back(Vec3f(0.80, 0.35, 0.35));
                b.push_back(Vec3f(0.85, 0.41, 0.41));
                b.push_back(Vec3f(0.90, 0.52, 0.52));
                b.push_back(Vec3f(0.95, 0.65, 0.65));
                b.push_back(Vec3f(1.00, 1, 1));
            }
                break;
            default: return;
            }

            fillRgb(r_ptr.get(), r);
            fillRgb(g_ptr.get(), g);
            fillRgb(b_ptr.get(), b);

            pseudo_color_map[0].swap(r);
            pseudo_color_map[1].swap(g);
            pseudo_color_map[2].swap(b);
        }

        void gray_to_pseudo(GRAY *in, RGB *out, const int &width, const int &height) {
            RGB *r = r_ptr.get();
            RGB *g = g_ptr.get();
            RGB *b = b_ptr.get();

            if( contrast == __no_contrast ) {
                for(int i = 0; i < height; i ++) {
                    for(int j = 0; j < width; j ++) {
                        *out ++ = r[in[width * i + j]];
                        *out ++ = g[in[width * i + j]];
                        *out ++ = b[in[width * i + j]];
                    }
                }
            }
            else {
                for(int i = 0; i < height; i ++) {
                    for(int j = 0; j < width; j ++) {
                        *out ++ = truncate(factor * (r[in[width * i + j]] - 128) + 128);
                        *out ++ = truncate(factor * (g[in[width * i + j]] - 128) + 128);
                        *out ++ = truncate(factor * (b[in[width * i + j]] - 128) + 128);
                    }
                }
            }
        }

        void yuv420p_to_pseudo(const YUV *in, RGB *out, const int &width, const int &height) {
            RGB *r = r_ptr.get();
            RGB *g = g_ptr.get();
            RGB *b = b_ptr.get();
            int offset = 0;
            for(int i = 0; i < height; i ++)
            {
                for(int j = 0; j < width; j ++)
                {
                    out[offset ++] = r[in[i * width + j]];
                    out[offset ++] = g[in[i * width + j]];
                    out[offset ++] = b[in[i * width + j]];
                }
            }
        }

        void yuv422_to_pseudo(YUV *in, RGB *out, const int &width, const int &height) {
            RGB *r = r_ptr.get();
            RGB *g = g_ptr.get();
            RGB *b = b_ptr.get();
            for(int i = 0; i < width * height * 2; i += 4) {
                *out ++ = r[in[i + 0]];
                *out ++ = g[in[i + 0]];
                *out ++ = b[in[i + 0]];

                *out ++ = r[in[i + 2]];
                *out ++ = g[in[i + 2]];
                *out ++ = b[in[i + 2]];
            }
        }

        void gray_to_rgb_pixel(const GRAY &gray, RGBPixel *rgb) {
            if( contrast > __no_contrast ) {
                rgb->r = truncate(factor * (r_ptr.get()[gray] - 128) + 128);
                rgb->g = truncate(factor * (g_ptr.get()[gray] - 128) + 128);
                rgb->b = truncate(factor * (b_ptr.get()[gray] - 128) + 128);
            }
            else {
                rgb->r = r_ptr.get()[gray];
                rgb->g = g_ptr.get()[gray];
                rgb->b = b_ptr.get()[gray];
            }
        }

        const vector<Vec3f> &pseudoColorMap(const int &index) {
            return pseudo_color_map[index];
        }

        ContrastLevel contrastLevel() { return contrast; }
        void setContrastLevel(const ContrastLevel &c) {
            contrast = c;
            factor = (259.0 * ((contrast * 51)  + 255.0)) / (255.0 * (259.0 - (contrast * 51)));
        }

    private:
        PseudoColorTable current_table;
        unique_ptr<RGB> r_ptr;
        unique_ptr<RGB> g_ptr;
        unique_ptr<RGB> b_ptr;
        ContrastLevel contrast;
        float factor;

        vector<Vec3f> pseudo_color_map[3];

        void fillRgb(unsigned char *rgb_ptr, const vector<Vec3f> &rgb_pseudo) {
            size_t idx = 0;
            for(int i = 0; i < RGB_SIZE; i ++)
            {
                float r = i / 255.0f;
                if( (r < 1.0)
                        && (r >= rgb_pseudo[idx + 1].x)
                        && ((idx + 1) < rgb_pseudo.size()) ) {
                    idx ++;
                }
                double prop = (r - rgb_pseudo[idx].x) / (rgb_pseudo[idx + 1].x - rgb_pseudo[idx].x);
                double val = rgb_pseudo[idx].z + (rgb_pseudo[idx + 1].y - rgb_pseudo[idx].z) * prop;
                rgb_ptr[i] = (int)(255 * val);
            }
        }

        int truncate(const int &value) {
            return (value > 255 ? 255 : (value < 0 ? 0 : value));
        }
    };

    unique_ptr<PseudoColor> pseudoColor;
};

#endif
