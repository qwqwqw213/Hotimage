#include "imageprovider.h"

#include "QDebug"
#include "QThread"
#include "QQueue"
#include "QMutex"

class ImageProviderPrivate
{
public:
    ImageProviderPrivate(ImageProvider *parent, const QString &url);
    ~ImageProviderPrivate();

    quint64 index;
    QString rawUrl;
    QString qmlUrl;

    QQueue<QImage> queue;
    QMutex mutex;

private:
    ImageProvider *f;
};

ImageProvider::ImageProvider(const QString &url)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , p(new ImageProviderPrivate(this, url))
{

}

ImageProvider::~ImageProvider()
{
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    p->index ++;
    if( !p->queue.isEmpty() ) {
        p->mutex.lock();
        QImage img = p->queue.dequeue();
        p->mutex.unlock();
        return img;
    }
    return QImage();
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    p->index ++;
    if( !p->queue.isEmpty() ) {
        p->mutex.lock();
        QImage img = p->queue.dequeue();
        p->mutex.unlock();
        return QPixmap::fromImage(img);
    }
    return QPixmap();
}

void ImageProvider::addQueue(QImage image)
{
//    m_image = image;
    p->mutex.lock();
    p->queue.enqueue(image);
    p->mutex.unlock();
}

void ImageProvider::clear()
{
    p->queue.clear();
}

QString ImageProvider::url()
{
    return p->rawUrl;
}

QString ImageProvider::qmlUrl()
{
    return p->qmlUrl + QString::number(p->index);
}

ImageProviderPrivate::ImageProviderPrivate(ImageProvider *parent, const QString &url)
{
    f = parent;


    index = 0;
    rawUrl = url;
    qmlUrl = QString("image://%1/").arg(url);

    queue.clear();
}

ImageProviderPrivate::~ImageProviderPrivate()
{
    queue.clear();
}
