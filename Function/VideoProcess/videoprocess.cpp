#include "videoprocess.h"

#include "thread"
#include "iostream"
#include "malloc.h"
#include "queue"
#include "mutex"
#include "condition_variable"

#include "QDebug"
#include "QThread"
#include "QMutex"

static int ffmpeg_init = 0;

class VideoProcessPrivate
{

public:
    explicit VideoProcessPrivate(VideoProcess *parent = nullptr);
    ~VideoProcessPrivate();

    std::string error;
    std::thread thread;
    std::mutex mutex;
    std::condition_variable condition;
    int status;

    QImage image;

//    std::mutex read_stream_mutex;
    QMutex read_stream_mutex;

    EncodeConfig config;
    QVector<QImage> encode_queue;
    struct _encode
    {
        qint64 second;
        AVFormatContext *fmtCnt;
        AVOutputFormat *outCnt;
        AVStream *stream;
        AVCodecContext *codecCnt;
        AVCodec *codec;
        AVFrame *frame;
        SwsContext *sws;
        AVPacket packet;
        int linesize[4];
    } *encode;


    void open_encode(const EncodeConfig &cfg);
    bool encode_frame(const uint8_t *data);
    bool flush_encoder(AVFormatContext *fmt_ctx, AVCodecContext *codecCnt, const unsigned int &stream_index);
    void release_encode();

    struct _decode
    {
        int video_index;
        int fps_count;
        double current_time;
        double total_time;
        int64_t cur_dts_ms;
        int seekFlag;
        AVFormatContext *fmtCnt;
        AVStream *stream;
        AVCodecContext *codecCnt;
        AVCodec *codec;
        AVFrame *frame;
        AVFrame *rgbFrame;
        SwsContext *sws;
        AVPacket packet;
        AVCodecParserContext *codecParserCnt;
    } *decode;

    void open_stream(const std::string &url, const int &msec);
    int find_stream(const char *url);
    int read_video_stream();
    void seek_stream(const int &msec);
    void jump_stream(const int &flag, const int &msec);
    void close_video_stream();

    int open_h264_decode();
    int send_h264_data(uint8_t *data, const int &size);
    void decode_packet(AVPacket *packet, const bool &flush);
    void release_decode();

    std::string format(const char *format, ...);

    class Stream
    {
    public:
        Stream() {}
        ~Stream() {
            mutex.lock();
            while (canTake()) {
                uint8_t *buf = NULL;
                int size;
                take(&buf, size);
                release(&buf);
            }
            mutex.unlock();
        }
        void push(uint8_t *buffer, const int &size) {
            uint8_t *buf = new uint8_t[size];
            memcpy(buf, buffer, size);
            mutex.lock();
            queue.push(std::make_tuple(buf, size));
            mutex.unlock();
        }
        bool canTake() { return !queue.empty(); }
        void take(uint8_t **buf, int &size) {
            if( !canTake() ) {
                return;
            }
            auto d = queue.front();
            (*buf) = std::get<0>(d);
            size = std::get<1>(d);
            mutex.lock();
            queue.pop();
            mutex.unlock();
        }
        void release(uint8_t **buf) {
            if( (*buf) == NULL ) {
                return;
            }
            delete [] (*buf);
            *buf = NULL;
        }
    private:
        std::queue<std::tuple<uint8_t *, int>> queue;
        std::mutex mutex;
    };
    Stream *h264_stream;

private:
    VideoProcess *f;
};

VideoProcess::VideoProcess(QObject *parent)
    : QObject(parent)
    , p(new VideoProcessPrivate(this))
{
    if( ffmpeg_init == 0 ) {
        ffmpeg_init = 1;
        avcodec_register_all();
        av_register_all();
    }
}

VideoProcess::~VideoProcess()
{

}

bool VideoProcess::openEncode(const EncodeConfig &config)
{
    p->open_encode(config);
    return true;
}

void VideoProcess::pushEncode(QImage img)
{
    if( p->encode ) {
        p->encode_queue.append(img);
        p->condition.notify_all();
    }
}

QString VideoProcess::recordTime()
{
    if( !p->encode ) {
        return QString("0:0");
    }

    quint64 sec = p->encode->second;
    if( sec >= 3600 ) {
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

QString VideoProcess::filePath()
{
    return p->config.filePath;
}

bool VideoProcess::closeEncode()
{
    if( p->status ) {
        p->status = __stop;
        p->condition.notify_all();

        p->thread.join();

        p->encode_queue.clear();
        emit statusChanged();
    }
    return true;
}

void VideoProcess::openStream(const std::string &url, const int &msec)
{
    p->open_stream(url, msec);
}

void VideoProcess::closeStream()
{
    p->close_video_stream();
}

QString VideoProcess::currentTime()
{
    if( p->decode ) {
        quint64 sec = (quint64)p->decode->current_time;
        if( sec < 3600 ) {
            return QString("%1:%2")
                    .arg(sec / 60, 2, 10, QLatin1Char('0'))
                    .arg(sec % 60, 2, 10, QLatin1Char('0'));
        }
        else {
            return QString("%1:%2:%3")
                    .arg(sec / 3600, 2, 10, QLatin1Char('0'))
                    .arg(sec / 60, 2, 10, QLatin1Char('0'))
                    .arg(sec % 60, 2, 10, QLatin1Char('0'));
        }
    }
    return QString("00:00");
}

QString VideoProcess::totalTime()
{
    if( p->decode ) {
        quint64 sec = (quint64)p->decode->total_time;
        if( sec < 3600 ) {
            return QString("%1:%2")
                    .arg(sec / 60, 2, 10, QLatin1Char('0'))
                    .arg(sec % 60, 2, 10, QLatin1Char('0'));
        }
        else {
            return QString("%1:%2:%3")
                    .arg(sec / 3600, 2, 10, QLatin1Char('0'))
                    .arg(sec / 60, 2, 10, QLatin1Char('0'))
                    .arg(sec % 60, 2, 10, QLatin1Char('0'));
        }
    }
    return QString("00:00");
}

int64_t VideoProcess::currentMescTime()
{
    if( !p->decode ) {
        return 0;
    }
    return p->decode->current_time * 1000;
}

int64_t VideoProcess::totalMsecTime()
{
    if( !p->decode ) {
        return 0;
    }
    return p->decode->total_time * 1000;
}

int VideoProcess::pausePlay()
{
    if( p->status == __running ) {
        p->status = __pause;
    }
    else if( p->status == __pause ) {
        p->status = __running;
        p->condition.notify_all();
    }
    return p->status;
}

void VideoProcess::seek(const int &msec)
{
    p->seek_stream(msec);
}

void VideoProcess::jump(const int &flag, const int &msec)
{
    p->jump_stream(flag, msec);
}

int VideoProcess::status()
{
    return p->status;
}

int VideoProcess::openDecode()
{
    return p->open_h264_decode();
}

void VideoProcess::closeDecode()
{
    if( p->decode ) {
        p->status = __stop;
        p->condition.notify_all();
        p->thread.join();
    }
//    p->release_decode();
}

int VideoProcess::pushPacket(uint8_t *data, const int &size)
{
    if( !data ) {
        return -1;
    }

    p->h264_stream->push(data, size);
    p->condition.notify_all();

//    return p->send_h264_data(buf, size);
    return 0;
}

std::string VideoProcess::lastError()
{
    return p->error;
}

QImage VideoProcess::image()
{
    return p->image;
}

VideoProcessPrivate::VideoProcessPrivate(VideoProcess *parent)
{
    f = parent;

    encode = NULL;
    decode = NULL;

    image = QImage();

    status = VideoProcess::__stop;
}

VideoProcessPrivate::~VideoProcessPrivate()
{

}

void VideoProcessPrivate::open_encode(const EncodeConfig &cfg)
{
    if( encode ) {
        return;
    }

    config = cfg;
    thread = std::thread([=](){
        int ret = 0;
        std::string str = config.filePath.toStdString();
        const char *filePath = str.c_str();
        std::string error("");
        AVRational fps;
        error = "";
        encode = (_encode *)malloc(sizeof (*encode));
        if( !encode ) {
            error = "malloc fail";
            goto FFMPEG_FAIL;
        }
        memset(encode, 0, sizeof (*encode));

        fps = {config.fps, 1};

        ret = avformat_alloc_output_context2(&encode->fmtCnt, NULL, NULL, filePath);
        if( ret < 0 ) {
            error = std::string("avformat_alloc_output_context2 fail, file path: ") + config.filePath.toStdString();
            goto FFMPEG_FAIL;
        }
        encode->outCnt = encode->fmtCnt->oformat;

        ret = avio_open2(&encode->fmtCnt->pb, filePath, AVIO_FLAG_WRITE, NULL, NULL);
        if( ret < 0 ) {
            error = "avio_open2 fail" + str;
            goto FFMPEG_FAIL;
        }

        encode->stream = avformat_new_stream(encode->fmtCnt, nullptr);
        if( !encode->stream ) {
            error = "avformat_new_stream fail";
            goto FFMPEG_FAIL;
        }
        encode->stream->time_base = av_inv_q(fps);

        encode->codecCnt = encode->stream->codec;
        encode->codecCnt->codec_id = encode->outCnt->video_codec;
        encode->codecCnt->codec_type = AVMEDIA_TYPE_VIDEO;
        encode->codecCnt->pix_fmt = AV_PIX_FMT_YUV420P;
        encode->codecCnt->width = config.width;
        encode->codecCnt->height = config.height;
        encode->codecCnt->time_base = av_inv_q(fps);
        encode->codecCnt->bit_rate = 500000;
        encode->codecCnt->gop_size = config.fps / 2;
        qDebug() << "encode codec_id:" << encode->codecCnt->codec_id;
        if (encode->codecCnt->codec_id == AV_CODEC_ID_H264)
        {
            encode->codecCnt->qmin = 10;
            encode->codecCnt->qmax = 51;
            encode->codecCnt->qcompress = 0.6;
        }
        if (encode->codecCnt->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            encode->codecCnt->max_b_frames = 2;
        }
        if (encode->codecCnt->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            encode->codecCnt->mb_decision = 2;
        }

        avcodec_parameters_from_context(encode->stream->codecpar, encode->codecCnt);

        // 查询编码器
        encode->codec = avcodec_find_encoder(encode->codecCnt->codec_id);
        if( !encode->codec ) {
            error = "avcodec_find_encoder fail";
            goto FFMPEG_FAIL;
        }

        if( (ret = avcodec_open2(encode->codecCnt, encode->codec, nullptr)) < 0) {
            error = "avcodec_open2 fail";
            goto FFMPEG_FAIL;
        }

        av_dump_format(encode->fmtCnt, 0, filePath, 1);

        encode->frame = av_frame_alloc();
        encode->frame->width = encode->codecCnt->width;
        encode->frame->height = encode->codecCnt->height;
        encode->frame->format = encode->codecCnt->pix_fmt;
        ret = av_image_alloc(encode->frame->data, encode->frame->linesize, config.width, config.height, AV_PIX_FMT_YUV420P, 1);
        if( ret < 0 ) {
            error = "av_image_alloc fail";
            goto FFMPEG_FAIL;
        }

        ret = avformat_write_header(encode->fmtCnt, nullptr);
        if( ret < 0 ) {
            error = "avformat_write_header fail";
            goto FFMPEG_FAIL;
        }

        // 获取原始像素linesize
        av_image_fill_linesizes(encode->linesize, config.encodePixel, config.width);
        for(int i = 0; i < 4; i ++) {
            qDebug() << "ffmpeg -> linesize:" << i << "," << encode->linesize[i] << "pixel:" << config.encodePixel;
        }

        // 图像转换context
        encode->sws = sws_getContext(config.width, config.height, config.encodePixel,
                                     config.width, config.height, encode->codecCnt->pix_fmt,
                                     SWS_FAST_BILINEAR, NULL, NULL, NULL);
        if( !encode->sws ) {
            error = "sws_getContext fail";
            goto FFMPEG_FAIL;
        }

        /* 写文件 */
        encode->frame->pts = 0;
        status = VideoProcess::__running;
        emit f->statusChanged();

        while (status == VideoProcess::__running) {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock);
            lock.unlock();

            while (encode_queue.size()) {
                QImage img = encode_queue.takeFirst();
                encode_frame(img.bits());
            }
        }

    FFMPEG_FAIL:
        if( ret < 0 ) {
            char err[512];
            av_make_error_string(err, 512, ret);
            std::string ffmpeg_err(err);
            error += (" ERROR: " + ffmpeg_err);
            emit f->error();
        }
        release_encode();
    });
}

bool VideoProcessPrivate::encode_frame(const uint8_t *data)
{
    if( !encode ) {
        return false;
    }

    qint64 sec = 0;
    int ret = 0;
    sws_scale(encode->sws,
              &data, encode->linesize, 0, config.height,
              encode->frame->data, encode->frame->linesize);

    av_init_packet(&encode->packet);
    ret = avcodec_send_frame(encode->codecCnt, encode->frame);
    if( ret < 0 ) {
        goto PUSH_END;
    }
    ret = avcodec_receive_packet(encode->codecCnt, &encode->packet);
    if( ret < 0 ) {
        goto PUSH_END;
    }

    encode->packet.stream_index = encode->stream->index;
    av_packet_rescale_ts(&encode->packet, encode->codecCnt->time_base, encode->stream->time_base);
    encode->packet.pos = -1;
    ret = av_interleaved_write_frame(encode->fmtCnt, &encode->packet);
    if( ret < 0 ) {
        goto PUSH_END;
    }

    encode->frame->pts ++;
    sec = encode->frame->pts / config.fps;
    if( sec != encode->second ) {
        encode->second = sec;
        emit f->recordTimeChanged();
    }

PUSH_END:
    av_packet_unref(&encode->packet);
    return ret;
}

bool VideoProcessPrivate::flush_encoder(AVFormatContext *fmt_ctx, AVCodecContext *codecCnt, const unsigned int &stream_index)
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

void VideoProcessPrivate::release_encode()
{
    if( !encode ) {
        return;
    }
    if( encode->frame )
    {
        bool flag = (encode->frame->pts > 0);
        if( flag )
        {
            if( flush_encoder(encode->fmtCnt, encode->codecCnt, 0) ) {
                av_write_trailer(encode->fmtCnt);
            }
        }
    }

    if( encode->sws ) {
        sws_freeContext(encode->sws);
    }
    if( encode->codecCnt ) {
       avcodec_close(encode->codecCnt);
    }
    if( encode->frame ) {
        av_frame_free(&encode->frame);
    }
    if( encode->fmtCnt ) {
        avio_close(encode->fmtCnt->pb);
        avformat_free_context(encode->fmtCnt);
    }

    free(encode);
    encode = NULL;

    qDebug() << "close encode";
}

void VideoProcessPrivate::open_stream(const std::string &url, const int &msec)
{
    if( decode ) {
        return;
    }

    thread = std::thread([=](){
        int res = 0;
        decode = (_decode *)malloc(sizeof (*decode));
        if( !decode ) {
            error = "decode mallo fail";
            res = -1;
        }

        if( res == 0 )
        {
            memset(decode, 0, sizeof (*decode));
            decode->cur_dts_ms = msec;
            res = find_stream(url.c_str());
            if( res == 0 ) {
                read_video_stream();
            }
        }

        if( res < 0 ) {
            emit f->error();
        }
    });
}

int VideoProcessPrivate::find_stream(const char *url)
{
    int res = avformat_open_input(&decode->fmtCnt, url, NULL, NULL);
    if( res < 0 ) {
        error = "open input fail, url:" + std::string(url);
        return -1;
    }

    res = avformat_find_stream_info(decode->fmtCnt, NULL);
    if( res < 0 ) {
        error = "find stream info fail";
        return -1;
    }

    qDebug() << "stream size:" << decode->fmtCnt->nb_streams;
    for(uint32_t i = 0; i < decode->fmtCnt->nb_streams; i ++) {
        AVStream *stream = decode->fmtCnt->streams[i];
        qDebug() << "stream type:" << stream->codec->codec_type;
        if( stream->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
            decode->stream = stream;
            decode->video_index = i;
            decode->codecCnt = stream->codec;
            decode->fps_count = stream->nb_frames;
            decode->total_time = decode->fmtCnt->duration / 1000000.0;
            qDebug() << "- stream info - \n"
                     << "   fps:" << stream->nb_frames << "\n"
                     << "   total time:" << decode->total_time << "\n";
            break;
        }
    }

    if( !decode->codecCnt ) {
        error = "find video codec fail";
        return -1;
    }

    decode->codec = avcodec_find_decoder(decode->codecCnt->codec_id);
    if( !decode->codec ) {
        error = "find decoder fail, codec id: " + std::to_string(decode->codecCnt->codec_id);
        return -1;
    }

    res = avcodec_open2(decode->codecCnt, decode->codec, NULL);
    if( res < 0 ) {
        error = "codec open fail";
        return -1;
    }

    decode->frame = av_frame_alloc();
    if( !decode->frame ) {
        error = "alloc frame error";
        return -1;
    }

    decode->rgbFrame = av_frame_alloc();
    if( !decode->rgbFrame ) {
        error = "alloc rgb frame error";
        return -1;
    }
    decode->rgbFrame->width = -1;
    decode->rgbFrame->height = -1;

    return res;
}

static QMutex seek_mutex;

int VideoProcessPrivate::read_video_stream()
{
    if( !decode->fmtCnt ) {
        error = "avformat context is null";
        return -1;
    }

    int res;
    qDebug() << "- codec info -\n"
             << "   width:" << decode->codecCnt->width
             << "   height:" << decode->codecCnt->height
             << "   pixel format:" << decode->codecCnt->pix_fmt;
    decode->sws = sws_getContext(decode->codecCnt->width, decode->codecCnt->height, decode->codecCnt->pix_fmt,
                                 decode->codecCnt->width, decode->codecCnt->height, AV_PIX_FMT_RGB24,
                                 SWS_FAST_BILINEAR, NULL, NULL, NULL);

    decode->rgbFrame->width = decode->codecCnt->width;
    decode->rgbFrame->height = decode->codecCnt->height;
    decode->rgbFrame->format = AV_PIX_FMT_RGB24;
    res = av_image_alloc(decode->rgbFrame->data, decode->rgbFrame->linesize,
                         decode->rgbFrame->width, decode->rgbFrame->height,
                         AV_PIX_FMT_RGB24, 1);
    if( res < 0 ) {
        error = "alloc rgb frame data fail";
        return -1;
    }

    image = QImage(*decode->rgbFrame->data,
                   decode->rgbFrame->width, decode->rgbFrame->height,
                   QImage::Format_RGB888);

    av_init_packet(&decode->packet);
    status = VideoProcess::__running;
    emit f->statusChanged();

//    decode->old_dts_ms = decode->total_time / 2.0 * AV_TIME_BASE;
    int64_t seek_usec = decode->cur_dts_ms * 1000;
    int old_dts_ms = decode->cur_dts_ms;
    av_seek_frame(decode->fmtCnt, -1, seek_usec, AVSEEK_FLAG_BACKWARD);

    qDebug() << QThread::currentThreadId() << "start play stream";
    while (status > VideoProcess::__stop)
    {
        // 暂停视频
        if( status == VideoProcess::__pause )
        {
            // 视频暂停状态更新
            emit f->statusChanged();
            qDebug() << "stream puase";
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock);
            lock.unlock();
            // 恢复播放状态更新
            emit f->statusChanged();
        }

        read_stream_mutex.lock();
        res = av_read_frame(decode->fmtCnt, &decode->packet);

        if( res < 0 )
        {
            status = VideoProcess::__stop;
            decode->current_time = decode->total_time;
            emit f->frame(image);

            qDebug() << "read stream finished";
            emit f->statusChanged();
            read_stream_mutex.unlock();
            break;
        }
        if( decode->packet.stream_index == decode->video_index )
        {
            res = avcodec_send_packet(decode->codecCnt, &decode->packet);
            if( res < 0 ) {
                read_stream_mutex.unlock();
                continue;
            }
            res = avcodec_receive_frame(decode->codecCnt, decode->frame);
            if( res < 0 ) {
                read_stream_mutex.unlock();
                continue;
            }
            int64_t cur_dts = decode->packet.dts;
            decode->cur_dts_ms = cur_dts * av_q2d(decode->stream->time_base) * 1000;
            decode->current_time = cur_dts * av_q2d(decode->stream->time_base);

            int wait_ms = decode->cur_dts_ms - old_dts_ms;
            if( decode->seekFlag ) {
                decode->seekFlag = 0;
                wait_ms = -1;
            }
            old_dts_ms = decode->cur_dts_ms;
            read_stream_mutex.unlock();

            sws_scale(decode->sws,
                      decode->frame->data, decode->frame->linesize, 0, decode->frame->height,
                      decode->rgbFrame->data, decode->rgbFrame->linesize);

            emit f->frame(image);

            if( wait_ms > 0 ) {
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
            }
        }
        else {
            read_stream_mutex.unlock();
        }
    }
    av_packet_unref(&decode->packet);

    image = QImage();

    return 0;
}

void VideoProcessPrivate::seek_stream(const int &msec)
{
    if( !decode && status > VideoProcess::__stop ) {
        return;
    }

    read_stream_mutex.lock();
    int64_t usec = msec * 1000;
    int ret = av_seek_frame(decode->fmtCnt, -1, usec, AVSEEK_FLAG_BACKWARD);
    if( ret >= 0 ) {
        decode->seekFlag = 1;
        avcodec_flush_buffers(decode->codecCnt);
    }
    read_stream_mutex.unlock();
}

void VideoProcessPrivate::jump_stream(const int &flag, const int &msec)
{
    if( !decode && status > VideoProcess::__stop ) {
        return;
    }

    int jump_usec = msec * 1000;

    read_stream_mutex.lock();
    int64_t usec = decode->cur_dts_ms * 1000;
    if( flag == 0 ) {
        usec -= jump_usec;
        if( usec < 0 ) {
            usec = 0;
        }
    }
    else {
        usec += jump_usec;
        if( usec > decode->total_time * AV_TIME_BASE ) {
            usec = decode->total_time * AV_TIME_BASE;
        }
    }
    int ret = av_seek_frame(decode->fmtCnt, -1, usec, AVSEEK_FLAG_BACKWARD);
    if( ret >= 0 ) {
        decode->seekFlag = 1;
        avcodec_flush_buffers(decode->codecCnt);
    }
    read_stream_mutex.unlock();
}

void VideoProcessPrivate::close_video_stream()
{
    if( !decode ) {
        return;
    }

    if( thread.joinable() ) {
        qDebug() << "close stream";

        status = VideoProcess::__stop;
        condition.notify_all();

        thread.join();

        if( decode->fmtCnt ) {
            avformat_close_input(&decode->fmtCnt);
        }

        if( decode->frame ) {
            av_frame_free(&decode->frame);
        }

        if( decode->rgbFrame ) {
            av_frame_free(&decode->rgbFrame);
        }

        if( decode->sws ) {
            sws_freeContext(decode->sws);
        }

        free(decode);
        decode = NULL;
    }
}

int VideoProcessPrivate::open_h264_decode()
{
    if( decode ) {
        return 0;
    }

    thread = std::thread([=](){
        decode = (_decode *)malloc(sizeof (*decode));
        if( !decode ) {
            qDebug() << "decode malloc fail";
            emit f->error();
            return;
        }
        memset(decode, 0, sizeof (*decode));

        h264_stream = new VideoProcessPrivate::Stream;

        // find decoder
        decode->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if( !decode->codec ) {
            error = "find h264 decoder fail";
            emit f->error();
            return;
        }

        // init code context
        decode->codecCnt = avcodec_alloc_context3(decode->codec);
        if( !decode->codecCnt ) {
            error = "alloc code context fail";
            emit f->error();
            return;
        }

        // init parser context
        decode->codecParserCnt = av_parser_init(AV_CODEC_ID_H264);
        if( !decode->codecParserCnt ) {
            error = "parser init fail";
            emit f->error();
            return;
        }

        if( decode->codec->capabilities & AV_CODEC_CAP_TRUNCATED ) {
            decode->codecCnt->flags |= AV_CODEC_FLAG_TRUNCATED;
        }

        // open code
        int res = avcodec_open2(decode->codecCnt, decode->codec, NULL);
        if( res < 0 ) {
            error = "open code fail";
            emit f->error();
            return;
        }

        decode->frame = av_frame_alloc();
        if( !decode->frame ) {
            error = "alloc frame error";
            emit f->error();
            return;
        }

        decode->rgbFrame = av_frame_alloc();
        if( !decode->rgbFrame ) {
            error = "alloc rgb frame error";
            emit f->error();
            return;
        }
        decode->rgbFrame->width = -1;
        decode->rgbFrame->height = -1;

        status = VideoProcess::__running;
        while (status == VideoProcess::__running) {
            if( h264_stream->canTake() ) {
                uint8_t *buf = NULL;
                int size;
                h264_stream->take(&buf, size);
                if( send_h264_data(buf, size) == 0 ) {
                    h264_stream->release(&buf);
                }
            }

            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock);
            lock.unlock();
        }

        release_decode();
    });
    return 0;
}


int VideoProcessPrivate::send_h264_data(uint8_t *data, const int &size)
{
    if( !decode ) {
        return -1;
    }

    uint8_t *packet_buf = NULL;
    int packet_size = 0;
    int len = av_parser_parse2(decode->codecParserCnt, decode->codecCnt,
                               &packet_buf, &packet_size,
                               data, size,
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

    if( len > 0 )
    {
        av_init_packet(&decode->packet);
        decode->packet.data = data;
        decode->packet.size = size;

        decode_packet(&decode->packet, 0);

        av_packet_unref(&decode->packet);
    }

//    int in_len = size;
//    while (in_len) {
//        int len = av_parser_parse2(decode->codecParserCnt, decode->codecCnt,
//                                   &decode->packet.data, &decode->packet.size,
//                                   data, size,
//                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
////        qDebug() << decode->codecParserCnt->pict_type << AV_PICTURE_TYPE_I << AV_PICTURE_TYPE_P << AV_PICTURE_TYPE_B;
//        in_len -= len;
//        data += len;

//        if( decode->packet.size == 0 ) {
//            continue;
//        }

//        int got_picture;
//        res = avcodec_decode_video2(decode->codecCnt, decode->frame,
//                                    &got_picture, &decode->packet);
//        if( res < 0 ) {
//            break;
//        }

//        if( got_picture ) {
//            // init rgb frame
//            if( decode->rgbFrame->width != decode->codecCnt->width ) {
//                decode->rgbFrame->width = decode->codecCnt->width;
//                decode->rgbFrame->height = decode->codecCnt->height;
//                av_image_alloc(decode->rgbFrame->data, decode->rgbFrame->linesize,
//                               decode->codecCnt->width, decode->codecCnt->height,
//                               AV_PIX_FMT_RGB24, 1);
//            }

//            // init sws context
//            if( !decode->sws ) {
//                decode->sws = sws_getContext(decode->frame->width, decode->frame->height, decode->codecCnt->pix_fmt,
//                                             decode->codecCnt->width, decode->codecCnt->height, AV_PIX_FMT_RGB24,
//                                             SWS_FAST_BILINEAR, NULL, NULL, NULL);
//            }

//            sws_scale(decode->sws,
//                      decode->frame->data, decode->frame->linesize, 0, decode->frame->height,
//                      decode->rgbFrame->data, decode->rgbFrame->linesize);

//            QImage img = QImage(*decode->rgbFrame->data,
//                                decode->rgbFrame->width, decode->rgbFrame->height,
//                                QImage::Format_RGB888);
//            emit f->frame(img);
//        }
//    }

    return 0;
}

void VideoProcessPrivate::decode_packet(AVPacket *packet, const bool &flush)
{
    int gof_frame = 0;
    int res = 0;
    do {
        res = avcodec_decode_video2(decode->codecCnt, decode->frame,
                                    &gof_frame, packet);
        if( res < 0 ) {
            return;
        }

        if( gof_frame )
        {
            if( decode->rgbFrame->width != decode->frame->width )
            {
                qDebug() << "rgb frame init";
                decode->rgbFrame->width = decode->frame->width;
                decode->rgbFrame->height = decode->frame->height;
                av_image_alloc(decode->rgbFrame->data, decode->rgbFrame->linesize,
                               decode->frame->width, decode->frame->height,
                               AV_PIX_FMT_RGB24, 1);

                decode->sws = sws_getContext(decode->frame->width, decode->frame->height, decode->codecCnt->pix_fmt,
                                             decode->frame->width, decode->frame->height, AV_PIX_FMT_RGB24,
                                             SWS_FAST_BILINEAR, NULL, NULL, NULL);
            }

            sws_scale(decode->sws,
                      decode->frame->data, decode->frame->linesize, 0, decode->frame->height,
                      decode->rgbFrame->data, decode->rgbFrame->linesize);

            QImage img = QImage(*decode->rgbFrame->data,
                                decode->rgbFrame->width, decode->rgbFrame->height,
                                QImage::Format_RGB888);
            emit f->frame(img);
        }
    } while(flush && gof_frame > 0);
}

void VideoProcessPrivate::release_decode()
{
    if( !decode ) {
        return;
    }

    if( decode->fmtCnt ) {
        avformat_close_input(&decode->fmtCnt);
    }

    if( decode->codecCnt ) {
       avcodec_close(decode->codecCnt);
    }

    if( decode->frame ) {
        av_frame_free(&decode->frame);
    }

    if( decode->rgbFrame ) {
        av_frame_free(&decode->rgbFrame);
    }

    if( decode->sws ) {
        sws_freeContext(decode->sws);
    }

    if( decode->codecParserCnt ) {
        av_parser_close(decode->codecParserCnt);
    }

    if( h264_stream ) {
        delete h264_stream;
        h264_stream = NULL;
    }

    free(decode);
    decode = NULL;
}

std::string VideoProcessPrivate::format(const char *format, ...)
{
    char buf[1024];
//    va_list args;
//    va_start(args, format);
//    vsprintf_s(buf, sizeof(buf), format, args);
//    va_end(args);

//    std::string str(buf);
//    return str;
    return "";
}
