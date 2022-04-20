#include "imageprovider.h"

#include "QDebug"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
ImageProvider::ImageProvider(const bool &realtime)
    : QQuickImageProvider(QQmlImageProviderBase::Image)
#else
ImageProvider::ImageProvider(const bool &realtime)
    : QObject(nullptr)
    , QQuickImageProvider(QQmlImageProviderBase::Image)
#endif
{
    m_realtime = realtime;
    m_url.clear();

    m_image = QImage();
}

ImageProvider::~ImageProvider()
{
    qDebug() << "image provider release";
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
//    qDebug() << "request:" << m_image.isNull() << m_image.width() << m_image.height();
    if( m_queue.size() < 1 ) {
        return m_image;
    }
    return m_queue.dequeue();
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    return QPixmap();
}

void ImageProvider::append(QImage &img)
{
    if( !hasUrl() ) {
        return;
    }
    if( img.isNull() ) {
        qDebug() << "ERROR: provider append null image";
        return;
    }

    if( m_realtime ) {
        m_image.swap(img);
    }
    else {
        QImage i;
        i.swap(img);
        m_queue.enqueue(i);
    }
    emit imageEnqueue();
}

QString ImageProvider::url()
{
    return m_url + QString::number(QDateTime::currentMSecsSinceEpoch());
}

QString ImageProvider::freezeUrl()
{
    return m_image.isNull() ? "" : (m_url + QString::number(QDateTime::currentMSecsSinceEpoch()));
}

bool ImageProvider::setUrl(QQmlApplicationEngine *e, const QString &path)
{
    if( m_url.isEmpty() && e != nullptr ) {
        e->addImageProvider(path, this);
        m_url = QString("image://%1/").arg(path);
        return true;
    }
    return false;
}

bool ImageProvider::hasUrl() {
    return !m_url.isEmpty();
}

bool ImageProvider::canRead()
{
//    return (m_queue.size() > 0);
    return !m_image.isNull();
}

void ImageProvider::clear()
{
    m_image = QImage();
    emit imageEnqueue();
}
