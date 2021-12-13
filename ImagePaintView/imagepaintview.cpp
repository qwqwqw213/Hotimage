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
    painter->drawPixmap(0, 0,
                        this->width(), this->height(),
                        QPixmap::fromImage(m_image));
}

void ImagePaintView::openStream(const QString &path)
{
    if( !decode->status() ) {
        QString file = path.right(path.length() - QString("file:///").length());
        qDebug() << "open video:" << file;
        decode->openStream(file.toStdString());
    }
}

void ImagePaintView::closeStream()
{
    decode->closeStream();
}
