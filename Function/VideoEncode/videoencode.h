#ifndef VIDEOENCODE_H
#define VIDEOENCODE_H

#include "QThread"
#include "QImage"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

typedef struct _tRecordConfig
{
//    char filePath[512];
    QString filePath;
    int width;
    int height;
    int fps;
    AVPixelFormat encodePixel;

    _tRecordConfig &operator=(const _tRecordConfig &c)
    {
        this->filePath = c.filePath;
        this->width = c.width;
        this->height = c.height;
        this->fps = c.fps;
        this->encodePixel = c.encodePixel;
        return *this;
    }
} RecordConfig;

class VideoEncodePrivate;
class VideoEncode : public QThread
{
    Q_OBJECT

public:
    explicit VideoEncode(QObject *parent = nullptr);
    ~VideoEncode();

    bool encoding();
    bool startEncode(const RecordConfig &config);
    bool stopEncode();

    void push(QImage img);

    inline AVPixelFormat pixel(const int &QImageFormat);

    // 返回编码视频时间
    // 格式: hh:mm:ss
    QString recordTime();

    QString error();

protected:
    void run() override;

private:
    friend class VideoEncodePrivate;
    QScopedPointer<VideoEncodePrivate> p;

Q_SIGNALS:
    void encodeFail();
    void recordTimeChanged();
};

inline AVPixelFormat VideoEncode::pixel(const int &QImageFormat)
{
    switch (QImageFormat) {
    case QImage::Format_RGB32: { return AVPixelFormat::AV_PIX_FMT_0RGB; }
    case QImage::Format_RGB888: { return AVPixelFormat::AV_PIX_FMT_RGB24; }
    default: { return AVPixelFormat::AV_PIX_FMT_NONE; }
    }
}

#endif // VIDEOENCODE_H
