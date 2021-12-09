#include "imageview.h"

#include "QDebug"
#include "QPainter"
#include "QPixmap"

ImageView::ImageView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    m_camera = nullptr;
    m_image = QImage();
}

ImageView::~ImageView()
{

}

TcpCamera * ImageView::camera()
{
    return m_camera;
}

void ImageView::setCamera(TcpCamera *camera)
{
    if( camera ) {
        qDebug() << "set camera";
        m_camera = camera;
        QObject::connect(m_camera, static_cast<void (TcpCamera::*)(QImage)>(&TcpCamera::videoFrame),
                         this, [=](QImage image){
            m_image.swap(image);
            update();
        }, Qt::QueuedConnection);
    }
}

void ImageView::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0,
                        this->width(), this->height(),
                        QPixmap::fromImage(m_image));
}
