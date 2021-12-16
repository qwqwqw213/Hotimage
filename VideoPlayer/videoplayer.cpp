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

    decode = new VideoProcess(this);
//    QObject::connect(decode, static_cast<void (VideoProcess::*)(QImage)>(&VideoProcess::frame),
//                     this, &VideoPlayer::updateImage, Qt::QueuedConnection);

    QObject::connect(decode, static_cast<void (VideoProcess::*)(QImage)>(&VideoProcess::frame),
                     [=](QImage img){
        videoProvider->add(img);
        emit frameUpdate();
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
        }
        emit playStatusChanged();
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
    if( !decode->status() ) {
        m_w = w;
        m_h = h;
        m_x = (this->width() - m_w) / 2.0;
        m_y = (this->height() - m_h) / 2.0;
        m_index = index;
        emit imageSizeChanged();
        decode->openStream(path.toStdString());
    }
}

void VideoPlayer::pause()
{
    decode->pause();
}

void VideoPlayer::closeStream()
{
    decode->closeStream();
    videoProvider->release();
    m_image = QImage();
    m_index = -1;
    update();
    emit playStatusChanged();
}

int VideoPlayer::playing()
{
    return decode->status();
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
