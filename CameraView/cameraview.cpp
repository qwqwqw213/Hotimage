#include "cameraview.h"

#include "QPainter"
#include "QDebug"

CameraView::CameraView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    QObject::connect(this, &CameraView::refresh,
                     this, [=](){ update(); },
                     Qt::QueuedConnection);
}

CameraView::~CameraView()
{

}

bool CameraView::canRead()
{
    return !m_img.isNull();
}

void CameraView::refreshView(QImage &img)
{
    bool flag = canRead();
    m_img.swap(img);
    if( !flag || m_img.isNull() ) {
        emit canReadChanged();
    }
    emit refresh();
}

void CameraView::paint(QPainter *painter)
{
    if( painter && !m_img.isNull() ) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(0, 0, this->width(), this->height(),
                            QPixmap::fromImage(m_img));
    }
}
