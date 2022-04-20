#include "videoplayer.h"

#include "queue.hpp"

#include "QDebug"
#include "QPainter"
#include "QPixmap"

#include "QElapsedTimer"
#include "QQmlApplicationEngine"
#include "QQuickImageProvider"

class VideoPlayerProvider : public QQuickImageProvider
{
public:
    VideoPlayerProvider()
        : QQuickImageProvider(QQmlImageProviderBase::Image)
        , url("") { }
    ~VideoPlayerProvider() { }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
        Q_UNUSED(id)
        Q_UNUSED(size)
        Q_UNUSED(requestedSize)
        return images.dequeue();
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
        Q_UNUSED(id)
        Q_UNUSED(size)
        Q_UNUSED(requestedSize)
        return QPixmap();
    }

    bool setUrl(QQmlApplicationEngine *e, const QString &path)
    {
        if( url.isEmpty() && e != nullptr ) {
            e->addImageProvider(path, this);
            url = QString("image://%1/").arg(path);
            return true;
        }
        return false;
    }

    bool canRead() { return images.size() > 0; }
    int append(QImage &img) { images.enqueue(img); return images.size(); }
    QString frameUrl() { return url + QString::number(QDateTime::currentMSecsSinceEpoch()); }
    void clear() { images.clear(); }

private:
    QString url;
    Queue<QImage> images;
};

class VideoPlayerPrivate
{
public:
    VideoPlayerPrivate(VideoPlayer *parent);
    ~VideoPlayerPrivate();

    VideoProcess *decode;
    int index;
    bool playing;
    VideoPlayerProvider *provider;

private:
    VideoPlayer *f;
};

VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent)
    , p(new VideoPlayerPrivate(this))
{
}

VideoPlayer::~VideoPlayer()
{
    closeStream();
}

void VideoPlayer::openStream(const QString &path, const int &index)
{
    if( p->decode->status() < 1 ) {
        p->index = index;
        p->decode->openStream(path.toStdString());
    }
}

void VideoPlayer::seek(const qreal &f)
{
    if( p->decode->status() > 0 ) {
        int msec = f * p->decode->totalMsecTime();
        p->decode->seek(msec);
    }
}

void VideoPlayer::playPause()
{
    p->decode->pausePlay();
}

void VideoPlayer::closeStream()
{
    p->decode->closeStream();
    p->provider->clear();
    p->index = -1;
    p->playing = false;
    emit playStatusChanged();
}

bool VideoPlayer::isValid()
{
    return (p->decode->status() > VideoProcess::__stop);
}

bool VideoPlayer::playing()
{
    return p->playing;
}

int VideoPlayer::playIndex()
{
    return p->index;
}

QString VideoPlayer::currentTime()
{
    return p->decode->currentTime();
}

QString VideoPlayer::totalTime()
{
    return p->decode->totalTime();
}

qreal VideoPlayer::progress()
{
    return p->decode->currentMescTime() / (qreal)p->decode->totalMsecTime();
}

QString VideoPlayer::frameUrl()
{

    return p->provider->canRead() ? p->provider->frameUrl() : "";
}

void VideoPlayer::setFrameUrl(QQmlApplicationEngine *e, const QString &path)
{
    p->provider->setUrl(e, path);
}

VideoPlayerPrivate::VideoPlayerPrivate(VideoPlayer *parent)
{
    f = parent;

    index = -1;
    playing = false;

    decode = new VideoProcess(f);
    QObject::connect(decode, static_cast<void (VideoProcess::*)(QImage)>(&VideoProcess::frame),
                     [=](QImage img){
        // qml image 第一次请求图片url的时候队列为空
        // 解码的时候第一张图片不发送更新消息
        // 否则暂停的时候无帧图片
        int size = provider->append(img);
        if( decode->status() == VideoProcess::__running
                && !playing ) {
            playing = true;
            emit f->playStatusChanged();
        }
        if( size > 1 ) {
            emit f->frameUpdate();
        }
    });

    QObject::connect(decode, &VideoProcess::error, f, [=](){
        f->closeStream();
    }, Qt::QueuedConnection);
    QObject::connect(decode, &VideoProcess::statusChanged, f, [=](){
        if( decode->status() == VideoProcess::__stop ) {
            f->closeStream();
        }
        else if( decode->status() == VideoProcess::__pause ) {
            playing = false;
            emit f->playStatusChanged();
        }
    }, Qt::QueuedConnection);
    provider = new VideoPlayerProvider;
}

VideoPlayerPrivate::~VideoPlayerPrivate()
{

}
