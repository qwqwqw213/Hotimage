#include "videoprocess.h"

#include "thread"
#include "iostream"
#include "mutex"
#include "malloc.h"
#include "queue"
#include "condition_variable"

#include "QDebug"

static int ffmpeg_init = 0;

class VideoProcessPrivate
{

public:
    explicit VideoProcessPrivate(VideoProcess *parent = nullptr);
    ~VideoProcessPrivate();

    EncodeConfig config;
    struct _encode
    {
        qint64 second;
        bool running;
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
    std::thread encode_thread;
    std::mutex encode_mutex;
    std::condition_variable encode_condition;
    std::string encode_error;
    QVector<QImage> encode_queue;

    void open_encode(const EncodeConfig &cfg);
    bool encode_frame(const uint8_t *data);
    bool flush_encoder(AVFormatContext *fmt_ctx, AVCodecContext *codecCnt, const unsigned int &stream_index);
    void release_encode();

    std::mutex stream_mutex;
    std::condition_variable stream_condition;
    struct _decode
    {
        int play;
        int video_index;
        int fps_count;
        double current_time;
        double total_time;
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
    std::string decode_error;
    std::thread read_stream_thread;
    void open_stream(const std::string &url);
    int find_stream(const char *url);
    int read_video_stream();
    void seek_stream(const int &sec);
    void close_video_stream();

    int open_h264_decode();
    int send_h264_data(uint8_t *data, const int &size);
    void decode_packet(AVPacket *packet, const bool &flush);
    void release_decode();

    std::string format(const char *format, ...);

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

bool VideoProcess::encodeRunning()
{
    return (p->encode != NULL);
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
        p->encode_condition.notify_all();
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
    if( p->encode ) {
        p->encode->running = false;
    }

    if( p->encode_thread.joinable() ) {
        p->encode_condition.notify_all();
        p->encode_thread.join();
    }
    return true;
}

std::string VideoProcess::encodeError()
{
    return p->encode_error;
}

void VideoProcess::openStream(const std::string &url)
{
    p->open_stream(url);
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

void VideoProcess::seek(const int &sec)
{
    p->seek_stream(sec);
}

int VideoProcess::openDecode()
{
    return p->open_h264_decode();
}

void VideoProcess::closeDecode()
{
    p->release_decode();
}

int VideoProcess::pushPacket(uint8_t *data, const int &size)
{
    if( !data ) {
        return -1;
    }
    return p->send_h264_data(data, size);
}

std::string VideoProcess::decodeError()
{
    return p->decode_error;
}

VideoProcessPrivate::VideoProcessPrivate(VideoProcess *parent)
{
    f = parent;

    encode = NULL;
    decode = NULL;
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
    encode_thread = std::thread([=](){
        int ret = 0;
        std::string str = config.filePath.toStdString();
        const char *filePath = str.c_str();
        std::string error("");
        AVRational fps;
        encode_error = "";
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
        encode->running = true;
        while (encode->running) {
            std::unique_lock<std::mutex> lock(encode_mutex);
            encode_condition.wait(lock);
            lock.unlock();

            while (encode_queue.size()) {
                QImage img = encode_queue.takeFirst();
                encode_frame(img.bits());
            }
        }

    FFMPEG_FAIL:
        release_encode();
        if( ret < 0 ) {
            char err[512];
            av_make_error_string(err, 512, ret);
            std::string ffmpeg_err(err);
            error += (" ERROR: " + ffmpeg_err);
            encode_error = error;
            emit f->error();
        }
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
}

void VideoProcessPrivate::open_stream(const std::string &url)
{
    if( decode || read_stream_thread.joinable() ) {
        return;
    }

    read_stream_thread = std::thread([=](){
        int res = 0;
        decode = (_decode *)malloc(sizeof (*decode));
        if( !decode ) {
            decode_error = "decode mallo fail";
            res = -1;
        }

        if( res == 0 )
        {
            memset(decode, 0, sizeof (*decode));
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
        decode_error = "open input fail, url:" + std::string(url);
        return -1;
    }

    res = avformat_find_stream_info(decode->fmtCnt, NULL);
    if( res < 0 ) {
        decode_error = "find stream info fail";
        return -1;
    }

    for(uint32_t i = 0; i < decode->fmtCnt->nb_streams; i ++) {
        AVStream *stream = decode->fmtCnt->streams[i];
        if( stream->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
            decode->stream = stream;
            decode->video_index = i;
            decode->codecCnt = stream->codec;
            decode->fps_count = stream->nb_frames;
            decode->total_time = decode->fmtCnt->duration / 1000000.0;
            break;
        }
    }

    if( !decode->codecCnt ) {
        decode_error = "find video codec fail";
        return -1;
    }

    decode->codec = avcodec_find_decoder(decode->codecCnt->codec_id);
    if( !decode->codec ) {
        decode_error = "find decoder fail";
        return -1;
    }

    res = avcodec_open2(decode->codecCnt, decode->codec, NULL);
    if( res < 0 ) {
        decode_error = "codec open fail";
        return -1;
    }

    decode->frame = av_frame_alloc();
    if( !decode->frame ) {
        decode_error = "alloc frame error";
        return -1;
    }

    decode->rgbFrame = av_frame_alloc();
    if( !decode->rgbFrame ) {
        decode_error = "alloc rgb frame error";
        return -1;
    }
    decode->rgbFrame->width = -1;
    decode->rgbFrame->height = -1;

    return res;
}

int VideoProcessPrivate::read_video_stream()
{
    if( !decode->fmtCnt ) {
        decode_error = "avformat context is null";
        return -1;
    }

    int res;
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
        decode_error = "alloc rgb frame data fail";
        return -1;
    }

    int fps = 0;
    QImage img = QImage(*decode->rgbFrame->data,
                        decode->rgbFrame->width, decode->rgbFrame->height,
                        QImage::Format_RGB888);

    av_init_packet(&decode->packet);
    qint64 start_ms = QDateTime::currentDateTime().toMSecsSinceEpoch();
    decode->play = 1;

    emit f->startPlay();

    while (decode->play)
    {
        res = av_read_frame(decode->fmtCnt, &decode->packet);
        if( res < 0 ) {
            decode->current_time = decode->total_time;
            emit f->frame(img);

            qDebug() << "read stream finished";
            emit f->readStreamFinished();

            std::unique_lock<std::mutex> lock(stream_mutex);
            stream_condition.wait(lock);
            lock.unlock();
        }
        if( decode->packet.stream_index == decode->video_index )
        {
            res = avcodec_send_packet(decode->codecCnt, &decode->packet);
            if( res < 0 ) {
                continue;
            }
            res = avcodec_receive_frame(decode->codecCnt, decode->frame);
            if( res < 0 ) {
                continue;
            }

            int64_t cur_dts = decode->packet.dts;
            int64_t cur_dts_ms = cur_dts * av_q2d(decode->stream->time_base) * 1000;

            fps ++;
            sws_scale(decode->sws,
                      decode->frame->data, decode->frame->linesize, 0, decode->frame->height,
                      decode->rgbFrame->data, decode->rgbFrame->linesize);




            while (1)
            {
                qint64 msec = QDateTime::currentDateTime().toMSecsSinceEpoch() - start_ms;
                if( msec > cur_dts_ms ) {
                    break;
                }
                decode->current_time = msec / 1000.0;
                emit f->frame(img);
            }
        }
    }
    av_packet_unref(&decode->packet);
    return 0;
}

void VideoProcessPrivate::seek_stream(const int &sec)
{
    if( !decode ) {
        return;
    }

    int64_t s = sec * av_q2d(decode->stream->time_base);
    int res = av_seek_frame(decode->fmtCnt, decode->video_index, s, AVSEEK_FLAG_ANY);
    qDebug() << "seek:" << res;

    stream_condition.notify_all();
}

void VideoProcessPrivate::close_video_stream()
{
    if( !decode ) {
        return;
    }

    stream_condition.notify_all();

    decode->play = 0;
    if( read_stream_thread.joinable() ) {
        read_stream_thread.join();
    }

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
}

int VideoProcessPrivate::open_h264_decode()
{
    if( decode ) {
        return 0;
    }

    decode = (_decode *)malloc(sizeof (*decode));
    if( !decode ) {
        qDebug() << "decode malloc fail";
        return -1;
    }
    memset(decode, 0, sizeof (*decode));

    // find decoder
    decode->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if( !decode->codec ) {
        decode_error = "find h264 decoder fail";
        return -1;
    }

    // init code context
    decode->codecCnt = avcodec_alloc_context3(decode->codec);
    if( !decode->codecCnt ) {
        decode_error = "alloc code context fail";
        return -1;
    }

    // init parser context
    decode->codecParserCnt = av_parser_init(AV_CODEC_ID_H264);
    if( !decode->codecParserCnt ) {
        decode_error = "parser init fail";
        return -1;
    }

    if( decode->codec->capabilities & AV_CODEC_CAP_TRUNCATED ) {
        decode->codecCnt->flags |= AV_CODEC_FLAG_TRUNCATED;
    }

    // open code
    int res = avcodec_open2(decode->codecCnt, decode->codec, NULL);
    if( res < 0 ) {
        decode_error = "open code fail";
        return -1;
    }

    decode->frame = av_frame_alloc();
    if( !decode->frame ) {
        decode_error = "alloc frame error";
        return -1;
    }

    decode->rgbFrame = av_frame_alloc();
    if( !decode->rgbFrame ) {
        decode_error = "alloc rgb frame error";
        return -1;
    }
    decode->rgbFrame->width = -1;
    decode->rgbFrame->height = -1;
    return 0;
}


int VideoProcessPrivate::send_h264_data(uint8_t *data, const int &size)
{
    if( !decode ) {
        return -1;
    }

    int res = 0;
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

    return res;
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
