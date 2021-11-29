#include "imageprovider.h"

#include "QDebug"

ImageProvider::ImageProvider(const QString &url)
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    m_image = QImage();

    m_requesTime = 0;

    m_url = url;
    m_qmlUrl = QString("image://%1/").arg(url);
}

ImageProvider::~ImageProvider()
{
    m_image = QImage();
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    m_requesTime ++;
    if( m_requesTime > 200 ) {
        m_requesTime = 0;
    }
    return m_image;
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    m_requesTime ++;
    if( m_requesTime > 200 ) {
        m_requesTime = 0;
    }
    return QPixmap::fromImage(m_image);
}

void ImageProvider::setEmptyRgbImage(const int &w, const int &h)
{
    m_image = QImage(w, h, QImage::Format_RGB888);
}

void ImageProvider::setImage(QImage image)
{
    m_image.swap(image);
}

uint8_t * ImageProvider::data()
{
    return m_image.bits();
}

QImage & ImageProvider::image()
{
    return m_image;
}

QString ImageProvider::url()
{
    return m_url;
}

QString ImageProvider::qmlUrl()
{
    return m_qmlUrl + QString::number(m_requesTime);
}

void ImageProvider::release()
{
    m_image = QImage();
}
