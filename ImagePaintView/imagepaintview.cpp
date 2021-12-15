#include "imagepaintview.h"

#include "QDebug"
#include "QPainter"
#include "QPixmap"

#include "QElapsedTimer"

ImagePaintView::ImagePaintView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    m_image = QImage();
    m_index = -1;

    decode = new VideoProcess(this);
//    QObject::connect(decode, static_cast<void (VideoProcess::*)(QImage)>(&VideoProcess::frame),
//                     this, &ImagePaintView::updateImage, Qt::QueuedConnection);

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
        if( !decode->status() ) {
            m_index = -1;
            decode->closeStream();
            videoProvider->release();
        }
        emit playStatusChanged();
    }, Qt::QueuedConnection);

    videoProvider = new VideoProvider;
}

ImagePaintView::~ImagePaintView()
{
    m_image = QImage();
    m_index = -1;
    decode->closeStream();
    videoProvider->release();
}

void ImagePaintView::updateImage(QImage image)
{
    m_image = image;
    update();
    emit frameUpdate();
}

void ImagePaintView::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(m_x, m_y,
                        m_w, m_h,
                        QPixmap::fromImage(m_image));
}

void ImagePaintView::openStream(const QString &path, const int &w, const int &h, const int &index)
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

void ImagePaintView::closeStream()
{
    decode->closeStream();
    videoProvider->release();
    m_image = QImage();
    m_index = -1;
    update();
    emit playStatusChanged();
}

bool ImagePaintView::playing()
{
    return decode->status();
}

int ImagePaintView::playIndex()
{
    return m_index;
}

int ImagePaintView::imageWidth()
{
    return m_w;
}

int ImagePaintView::imageHeight()
{
    return m_h;
}

QString ImagePaintView::currentTime()
{
    return decode->currentTime();
}

QString ImagePaintView::totalTime()
{
    return decode->totalTime();
}

qreal ImagePaintView::progress()
{
    return decode->currentMescTime() / (qreal)decode->totalMsecTime();
}

QString ImagePaintView::frameUrl()
{
    return videoProvider->qmlUrl();
}

VideoProvider * ImagePaintView::provider()
{
    return videoProvider;
}

QString ImagePaintView::providerUrl()
{
    return videoProvider->url();
}
