#include "imagepaintview.h"

#include "QDebug"
#include "QPainter"
#include "QPixmap"

#include "QElapsedTimer"

ImagePaintView::ImagePaintView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    m_image = QImage();

    decode = new VideoProcess(this);
    QObject::connect(decode, static_cast<void (VideoProcess::*)(QImage)>(&VideoProcess::frame),
                     this, &ImagePaintView::updateImage, Qt::QueuedConnection);
    QObject::connect(decode, &VideoProcess::error, this, [=](){
        decode->closeStream();
        emit playStatusChanged();
    }, Qt::QueuedConnection);
    QObject::connect(decode, &VideoProcess::statusChanged, this, [=](){
        if( !decode->status() ) {
            decode->closeStream();
        }
        emit playStatusChanged();
    }, Qt::QueuedConnection);
}

ImagePaintView::~ImagePaintView()
{
    m_image = QImage();
    decode->closeStream();
}

void ImagePaintView::updateImage(QImage image)
{
    m_image = image;
    update();
}

void ImagePaintView::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(m_x, m_y,
                        m_w, m_h,
                        QPixmap::fromImage(m_image));
}

void ImagePaintView::openStream(const QString &path, const int &w, const int &h)
{
    if( !decode->status() ) {
        m_w = w;
        m_h = h;
        m_x = (this->width() - m_w) / 2.0;
        m_y = (this->height() - m_h) / 2.0;
        decode->openStream(path.toStdString());
    }
}

void ImagePaintView::closeStream()
{
    decode->closeStream();
    m_image = QImage();
    update();
    emit playStatusChanged();
}

bool ImagePaintView::playing()
{
    return decode->status();
}
