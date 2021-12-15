#include "videoscanimage.h"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#include "QDebug"
#include "QQueue"

class VideoScanImagePrivate
{
public:
    VideoScanImagePrivate(VideoScanImage *parent, const QString &url);
    ~VideoScanImagePrivate();

    QString rawUrl;
    QString qmlUrl;

    QQueue<QImage> imageQueue;

    AVFormatContext *fmtCnt;
    AVStream *stream;
    AVCodecContext *codecCnt;
    AVCodec *codec;
    AVFrame *frame;
    AVFrame *rgbFrame;
    SwsContext *sws;

private:
    VideoScanImage *f;
};

VideoScanImage::VideoScanImage(const QString &url)
    : QQuickImageProvider(ImageType::Image)
    , p(new VideoScanImagePrivate(this, url))
{

}

VideoScanImage::~VideoScanImage()
{

}

QImage VideoScanImage::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    int i = id.toUInt();
    if( i < p->imageQueue.size() ) {
        return p->imageQueue.at(i);
    }
    return QImage();
}

QPixmap VideoScanImage::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    return QPixmap();
}

QString VideoScanImage::url()
{
    return p->rawUrl;
}

QString VideoScanImage::addQueue(const QString &path)
{
//    qDebug() << "video path:" << path;
    QString scanPath = p->qmlUrl + QString::number(p->imageQueue.size());

    std::string strUrl = path.toStdString();
    const char *url = strUrl.c_str();

    int videoIndex = -1;
    AVStream *stream = NULL;
    AVPacket packet;

    QImage img = QImage();

    int res = avformat_open_input(&p->fmtCnt, url, NULL, NULL);
    if( res < 0 ) {
//        return QString();
        goto FINISHED;
    }

    res = avformat_find_stream_info(p->fmtCnt, NULL);
    if( res < 0 ) {
        goto FINISHED;
    }

    for(uint32_t i = 0; i < p->fmtCnt->nb_streams; i ++) {
        stream = p->fmtCnt->streams[i];
        if( stream->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
            p->stream = stream;
            p->codecCnt = stream->codec;
            videoIndex = i;
            break;
        }
    }

    if( videoIndex < 0 ) {
        goto FINISHED;
    }

    p->codec = avcodec_find_decoder(p->codecCnt->codec_id);
    if( !p->codec ) {
        goto FINISHED;
    }

    res = avcodec_open2(p->codecCnt, p->codec, NULL);
    if( res < 0 ) {
        goto FINISHED;
    }

    p->frame = av_frame_alloc();
    if( !p->frame ) {
        goto FINISHED;
    }

    p->rgbFrame = av_frame_alloc();
    if( !p->rgbFrame ) {
        goto FINISHED;
    }

    p->sws = sws_getContext(p->codecCnt->width, p->codecCnt->height, p->codecCnt->pix_fmt,
                            p->codecCnt->width, p->codecCnt->height, AV_PIX_FMT_RGB24,
                            SWS_FAST_BILINEAR, NULL, NULL, NULL);

    p->rgbFrame->width = p->codecCnt->width;
    p->rgbFrame->height = p->codecCnt->height;
    p->rgbFrame->format = AV_PIX_FMT_RGB24;
    res = av_image_alloc(p->rgbFrame->data, p->rgbFrame->linesize,
                         p->rgbFrame->width, p->rgbFrame->height,
                         AV_PIX_FMT_RGB24, 1);
    if( res < 0 ) {
        goto FINISHED;
    }

    img = QImage(*p->rgbFrame->data,
                        p->rgbFrame->width, p->rgbFrame->height,
                        QImage::Format_RGB888);

    av_init_packet(&packet);
    while (true)
    {
        res = av_read_frame(p->fmtCnt, &packet);
        if( res < 0 ) {
            break;
        }
        if( packet.stream_index == videoIndex )
        {
            res = avcodec_send_packet(p->codecCnt, &packet);
            if( res < 0 ) {
                continue;
            }
            res = avcodec_receive_frame(p->codecCnt, p->frame);
            if( res < 0 ) {
                continue;
            }
            sws_scale(p->sws,
                      p->frame->data, p->frame->linesize, 0, p->frame->height,
                      p->rgbFrame->data, p->rgbFrame->linesize);
            break;
        }
    }
    av_packet_unref(&packet);

FINISHED:
    if( p->fmtCnt ) {
        avformat_close_input(&p->fmtCnt);
        p->fmtCnt = NULL;
    }

    if( p->frame ) {
        av_frame_free(&p->frame);
        p->frame = NULL;
    }

    if( p->rgbFrame ) {
        av_frame_free(&p->rgbFrame);
        p->rgbFrame = NULL;
    }

    if( p->sws ) {
        sws_freeContext(p->sws);
        p->sws = NULL;
    }


    if( img.isNull() ) {
        qDebug() << "get video scan error";
        return QString("");
    }
    p->imageQueue.enqueue(img);
//    qDebug() << "video scan url:" << scanPath;
    return scanPath;
}

VideoScanImagePrivate::VideoScanImagePrivate(VideoScanImage *parent, const QString &url)
{
    f = parent;

    rawUrl = url;
    qmlUrl = QString("image://%1/").arg(url);

    imageQueue.clear();

    fmtCnt = NULL;
    stream = NULL;
    codecCnt = NULL;
    codec = NULL;
    frame = NULL;
    rgbFrame = NULL;
    sws = NULL;
}

VideoScanImagePrivate::~VideoScanImagePrivate()
{

}
