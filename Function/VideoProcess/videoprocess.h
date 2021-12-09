#ifndef VIDEOPROCESS_H
#define VIDEOPROCESS_H

#include <QObject>
#include "QImage"
#include "QDateTime"
#include "QPainter"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

typedef struct _tEncodeConfig
{
    QString filePath;
    int width;
    int height;
    int fps;
    AVPixelFormat encodePixel;

    _tEncodeConfig &operator=(const _tEncodeConfig &c)
    {
        this->filePath = c.filePath;
        this->width = c.width;
        this->height = c.height;
        this->fps = c.fps;
        this->encodePixel = c.encodePixel;
        return *this;
    }
} EncodeConfig;

class VideoProcessPrivate;
class VideoProcess : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcess(QObject *parent = nullptr);
    ~VideoProcess();

    /*
     *  视频编码
     */
    bool openEncode(const EncodeConfig &config);
    void pushEncode(QImage img);
    QString recordTime();
    bool closeEncode();
    QString filePath();

    /*
     *  视频播放
     */
    void openStream(const std::string &url);
    void closeStream();
    QString currentTime();
    QString totalTime();
    int64_t totalMsecTime();
    int64_t currentMescTime();
    void seek(const int &sec);

    /*
     *  解码h264流
     */
    int openDecode();
    void closeDecode();
    int pushPacket(uint8_t *data, const int &size);

    /*
     *  解码器 编码器状态
     *  true  运行中
     *  false 未打开
     */
    bool status();
    std::string lastError();
    inline AVPixelFormat pixel(const int &QImageFormat);

private:
    friend class VideoProcessPrivate;
    QScopedPointer<VideoProcessPrivate> p;

Q_SIGNALS:
    void statusChanged();

    void recordTimeChanged();
    void error();

    void frame(QImage);
};

inline AVPixelFormat VideoProcess::pixel(const int &QImageFormat)
{
    switch (QImageFormat) {
    case QImage::Format_RGB32: { return AVPixelFormat::AV_PIX_FMT_0RGB; }
    case QImage::Format_RGB888: { return AVPixelFormat::AV_PIX_FMT_RGB24; }
    default: { return AVPixelFormat::AV_PIX_FMT_NONE; }
    }
}

#endif // VIDEOPROCESS_H
