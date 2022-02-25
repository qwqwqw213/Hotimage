#include "videoplayer.h"

#include "QDebug"
#include "QPainter"
#include "QPixmap"

#include "QElapsedTimer"

VideoPlayer::VideoPlayer(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    m_image = QImage();
    m_index = -1;
    m_status = __video_close;

    decode = new VideoProcess(this);
//    QObject::connect(decode, static_cast<void (VideoProcess::*)(QImage)>(&VideoProcess::frame),
//                     this, &VideoPlayer::updateImage, Qt::QueuedConnection);

    QObject::connect(decode, static_cast<void (VideoProcess::*)(QImage)>(&VideoProcess::frame),
                     [=](QImage img){
        // qml image 第一次请求图片url的时候队列为空
        // 解码的时候第一张图片不发送更新消息
        // 否则暂停的时候无帧图片
        int size = videoProvider->add(img);
        if( size > 1 && decode->status() == VideoProcess::__running ) {
            if( m_status == __video_close ) {
                m_status = __video_playing;
                emit playStatusChanged();
            }
            emit frameUpdate();
        }
    });

    QObject::connect(decode, &VideoProcess::error, this, [=](){
        decode->closeStream();
        videoProvider->release();
        m_index = -1;
        emit playStatusChanged();
    }, Qt::QueuedConnection);
    QObject::connect(decode, &VideoProcess::statusChanged, this, [=](){
        if( decode->status() == VideoProcess::__stop ) {
            m_index = -1;
            decode->closeStream();
            videoProvider->release();
            m_status = __video_close;
            emit playStatusChanged();
        }
        else if( decode->status() == VideoProcess::__pause ) {
            m_status = __video_pause;
            emit playStatusChanged();
        }
    }, Qt::QueuedConnection);

    videoProvider = new VideoProvider;
}

VideoPlayer::~VideoPlayer()
{
    m_image = QImage();
    m_index = -1;
    decode->closeStream();
    videoProvider->release();
}

void VideoPlayer::updateImage(QImage image)
{
    m_image = image;
    update();
    emit frameUpdate();
}

void VideoPlayer::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(m_x, m_y,
                        m_w, m_h,
                        QPixmap::fromImage(m_image));
}

void VideoPlayer::openStream(const QString &path, const int &w, const int &h, const int &index)
{
    if( decode->status() < 1 ) {
        m_w = w;
        m_h = h;
        m_x = (this->width() - m_w) / 2.0;
        m_y = (this->height() - m_h) / 2.0;
        m_index = index;
        emit imageSizeChanged();
        decode->openStream(path.toStdString());
    }
}

void VideoPlayer::seek(const qreal &f)
{
    if( decode->status() > 0 ) {
        int msec = f * decode->totalMsecTime();
        decode->seek(msec);
    }
}

void VideoPlayer::pause()
{
    int state = decode->pausePlay();
    if( state == VideoProcess::__pause ) {
        // 暂停后到下一次开始
        // QML Image类会请求一次图片路径
        // 这时候Provider无图片, 请求失败, 导致画面会闪一下
        // 暂停的时候需要补一帧图片到Provider
        videoProvider->add(decode->image());
    }
}

void VideoPlayer::closeStream()
{
    decode->closeStream();
    videoProvider->release();
    m_image = QImage();
    m_index = -1;
    m_status = __video_close;
    update();
    emit playStatusChanged();
}

int VideoPlayer::playing()
{
    return m_status;
}

int VideoPlayer::playIndex()
{
    return m_index;
}

int VideoPlayer::imageWidth()
{
    return m_w;
}

int VideoPlayer::imageHeight()
{
    return m_h;
}

QString VideoPlayer::currentTime()
{
    return decode->currentTime();
}

QString VideoPlayer::totalTime()
{
    return decode->totalTime();
}

qreal VideoPlayer::progress()
{
    return decode->currentMescTime() / (qreal)decode->totalMsecTime();
}

QString VideoPlayer::frameUrl()
{
    return videoProvider->qmlUrl();
}

VideoProvider * VideoPlayer::provider()
{
    return videoProvider;
}

QString VideoPlayer::providerUrl()
{
    return videoProvider->url();
}
