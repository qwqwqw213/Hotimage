#include "videoencode.h"

#include "QMutex"
#include "QWaitCondition"
#include "QPainter"
#include "QDebug"

class VideoEncodePrivate
{
public:
    explicit VideoEncodePrivate(VideoEncode *parent);
    ~VideoEncodePrivate();

    AVFormatContext *fmtCnt;
    AVOutputFormat *outCnt;
    AVStream *stream;
    AVCodecContext *codecCnt;
    AVCodec *codec;
    AVFrame *frame;
    SwsContext *sws;
    AVPacket packet;

    RecordConfig config;
    int linesize[4];
    QVector<QImage> imgQueue;

    QMutex mutex;
    QWaitCondition condition;

    quint64 second;

    QString error;

    bool encodec(const uint8_t *data);
    bool release();
    bool flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);

private:
    VideoEncode *f;
};


VideoEncode::VideoEncode(QObject *parent)
    : QThread(parent)
    , p(new VideoEncodePrivate(this))
{
    avcodec_register_all();
    av_register_all();
    qDebug() << "ffmpeg init";
}

VideoEncode::~VideoEncode()
{

}

bool VideoEncode::encoding()
{
    return this->isRunning();
}

bool VideoEncode::startEncode(const RecordConfig &config)
{
    if( !encoding() ) {
        p->config = config;
        this->start();
    }
    return true;
}

bool VideoEncode::stopEncode()
{
    if( encoding() ) {
        p->condition.wakeAll();
        this->requestInterruption();
        this->wait();
        p->imgQueue.clear();
    }
    return true;
}

void VideoEncode::push(QImage img)
{
    if( !encoding() ) {
        return;
    }
    p->imgQueue.push_back(img);
    p->condition.wakeAll();
}

QString VideoEncode::recordTime()
{
    quint64 sec = p->second;
    if( p->second >= 3600 ) {
        return QString("%1:%2:%3")
                .arg(sec / 3600)
                .arg((sec % 3600) / 60)
                .arg((sec % 3600) % 60);
    }
    else {
        return QString("%1:%2")
                .arg((sec % 3600) / 60)
                .arg((sec % 3600) % 60);
    }
}

QString VideoEncode::error()
{
    return p->error;
}

void VideoEncode::run()
{
    p->second = 0;
    p->fmtCnt = NULL;
    p->outCnt = NULL;
    p->stream = NULL;
    p->codecCnt = NULL;
    p->codec = NULL;
    p->frame = NULL;
    p->sws = NULL;
    AVRational fps = {p->config.fps, 1};


    std::string strPath = p->config.filePath.toStdString();
    const char *filePath = strPath.c_str();
    qDebug() << "ffmpeg -> encode path:" << filePath;

    int ret = avformat_alloc_output_context2(&p->fmtCnt, NULL, NULL, filePath);
    if( ret < 0 ) {
        qDebug() << "ffmpeg -> fail. avformat_alloc_output_context2";
        p->error = QString("avformat_alloc_output_context2 fail, file path: %1").arg(p->config.filePath);
        goto FFMPEG_FAIL;
    }
    p->outCnt = p->fmtCnt->oformat;

    ret = avio_open2(&p->fmtCnt->pb, filePath, AVIO_FLAG_WRITE, NULL, NULL);
    if( ret < 0 ) {
        qDebug() << "ffmpeg -> fail. avio_open2";
        goto FFMPEG_FAIL;
    }

    p->stream = avformat_new_stream(p->fmtCnt, nullptr);
    if( !p->stream ) {
        qDebug() << "ffmpeg -> fail. avformat_new_stream";
        goto FFMPEG_FAIL;
    }
    p->stream->time_base = av_inv_q(fps);

    p->codecCnt = p->stream->codec;
    p->codecCnt->codec_id = p->outCnt->video_codec;
    p->codecCnt->codec_type = AVMEDIA_TYPE_VIDEO;
    p->codecCnt->pix_fmt = AV_PIX_FMT_YUV420P;
    p->codecCnt->width = p->config.width;
    p->codecCnt->height = p->config.height;
    p->codecCnt->time_base = av_inv_q(fps);
    p->codecCnt->bit_rate = 500000;
    p->codecCnt->gop_size = p->config.fps / 2;
    if (p->codecCnt->codec_id == AV_CODEC_ID_H264)
    {
        p->codecCnt->qmin = 10;
        p->codecCnt->qmax = 51;
        p->codecCnt->qcompress = 0.6;
    }
    if (p->codecCnt->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        p->codecCnt->max_b_frames = 2;
    }
    if (p->codecCnt->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        p->codecCnt->mb_decision = 2;
    }

    avcodec_parameters_from_context(p->stream->codecpar, p->codecCnt);

    // 查询编码器
    p->codec = avcodec_find_encoder(p->codecCnt->codec_id);
    if( !p->codec ) {
        qDebug() << "ffmpeg -> fail. avcodec_find_encoder";
        goto FFMPEG_FAIL;
    }

    if( (ret = avcodec_open2(p->codecCnt, p->codec, nullptr)) < 0) {
        qDebug() << "ffmpeg -> fail. avcodec_open2";
        goto FFMPEG_FAIL;
    }

    av_dump_format(p->fmtCnt, 0, filePath, 1);

    p->frame = av_frame_alloc();
    p->frame->width = p->codecCnt->width;
    p->frame->height = p->codecCnt->height;
    p->frame->format = p->codecCnt->pix_fmt;
    ret = av_image_alloc(p->frame->data, p->frame->linesize, p->config.width, p->config.height, AV_PIX_FMT_YUV420P, 1);
    if( ret < 0 ) {
        qDebug() << "ffmpeg -> fail. av_image_alloc";
        goto FFMPEG_FAIL;
    }

    ret = avformat_write_header(p->fmtCnt, nullptr);
    if( ret < 0 ) {
        qDebug() << "ffmpeg -> fail. avformat_write_header";
        goto FFMPEG_FAIL;
    }

    // 获取原始像素linesize
    av_image_fill_linesizes(p->linesize, p->config.encodePixel, p->config.width);
    for(int i = 0; i < 4; i ++) {
        qDebug() << "ffmpeg -> linesize:" << i << "," << p->linesize[i] << "pixel:" << p->config.encodePixel;
    }

    // 图像转换context
    p->sws = sws_getContext(p->config.width, p->config.height, p->config.encodePixel,
                            p->config.width, p->config.height, p->codecCnt->pix_fmt,
                            SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if( !p->sws ) {
        qDebug() << "ffmpeg -> fail. sws_getContext";
        goto FFMPEG_FAIL;
    }

    /* 写文件 */
    p->frame->pts = 0;
    while (!this->isInterruptionRequested()) {
        p->mutex.lock();
        p->condition.wait(&p->mutex);
        p->mutex.unlock();

        while (p->imgQueue.size()) {
            QImage img = p->imgQueue.takeLast();
            p->encodec(img.bits());
        }
    }

    qDebug() << "record finished, rec sec:" << p->second;
FFMPEG_FAIL:
    if( !p->release() )
    {
        if( ret < 0 ) {
            char err[512];
            av_make_error_string(err, 512, ret);
            p->error = QByteArray(err);
            qDebug() << "ffmpeg -> ERROR:" << p->error;
        }
        emit encodeFail();
    }
}

VideoEncodePrivate::VideoEncodePrivate(VideoEncode *parent)
{
    f = parent;

    error = QString("");

    second = 0;
}

VideoEncodePrivate::~VideoEncodePrivate()
{

}

bool VideoEncodePrivate::encodec(const uint8_t *data)
{
    if( !codecCnt ) {
        return false;
    }

    quint64 sec = 0;
    int ret = 0;
    sws_scale(sws,
              &data, linesize, 0, config.height,
              frame->data, frame->linesize);

    av_init_packet(&packet);
    ret = avcodec_send_frame(codecCnt, frame);
    if( ret < 0 ) {
        goto PUSH_END;
    }
    ret = avcodec_receive_packet(codecCnt, &packet);
    if( ret < 0 ) {
        goto PUSH_END;
    }

    packet.stream_index = stream->index;
    av_packet_rescale_ts(&packet, codecCnt->time_base, stream->time_base);
    packet.pos = -1;
    ret = av_interleaved_write_frame(fmtCnt, &packet);
    if( ret < 0 ) {
        goto PUSH_END;
    }

    frame->pts ++;
    sec = frame->pts / config.fps;
    if( sec != second ) {
        second = sec;
        emit f->recordTimeChanged();
    }

PUSH_END:
    av_packet_unref(&packet);
    return ret;
}

bool VideoEncodePrivate::release()
{
    bool flag = false;
    if( frame )
    {
        flag = (frame->pts > 0);
        if( flag )
        {
            if( flush_encoder(fmtCnt, 0) ) {
                av_write_trailer(fmtCnt);
            }
        }
    }

    if( sws ) {
        sws_freeContext(sws);
    }
    if( codecCnt ) {
       avcodec_close(codecCnt);
    }
    if( frame ) {
        av_frame_free(&frame);
    }
    if( fmtCnt ) {
        avio_close(fmtCnt->pb);
        avformat_free_context(fmtCnt);
    }

    return flag;
}

bool VideoEncodePrivate::flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
    if( !(fmt_ctx->streams[stream_index]->codec->codec->capabilities & AV_CODEC_CAP_DELAY) ) {
        return 0;
    }

    int ret = 0;
    AVPacket enc_pkt;
    AVStream *_stream = fmt_ctx->streams[stream_index];
    AVCodecContext *_codecCnt = fmt_ctx->streams[stream_index]->codec;
    while (1)
    {
        enc_pkt.data = nullptr;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);

        ret = avcodec_send_frame(_codecCnt, nullptr);
        if( ret < 0 ) {
            break;
        }

        ret = avcodec_receive_packet(_codecCnt, &enc_pkt);
        if( ret < 0 ) {
            break;
        }

        enc_pkt.stream_index = stream_index;
        av_packet_rescale_ts(&enc_pkt, codecCnt->time_base, _stream->time_base);
        ret = av_interleaved_write_frame(fmt_ctx, &enc_pkt);
        if( ret < 0 ) {
            break;
        }
    }
    return true;
}

