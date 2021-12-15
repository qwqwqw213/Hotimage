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

//    QQueue<QImage> queue;
//    QMutex mutex;

    QImage image;

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
//    if( !p->queue.isEmpty() ) {
//        p->mutex.lock();
//        QImage img = p->queue.dequeue();
//        p->mutex.unlock();
//        qDebug() << p->queue.size();
//        return img;
//    }
    return p->image;
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    p->index ++;
//    if( !p->queue.isEmpty() ) {
//        p->mutex.lock();
//        QImage img = p->queue.dequeue();
//        p->mutex.unlock();
//        return QPixmap::fromImage(img);
//    }
    return QPixmap::fromImage(p->image);
}

//void ImageProvider::addQueue(QImage image)
//{
//    p->mutex.lock();
//    p->queue.enqueue(image);
//    p->mutex.unlock();
//}

//void ImageProvider::clear()
//{
//    p->queue.clear();
//}

QImage *ImageProvider::image(const int &w, const int &h)
{
    if( p->image.isNull() ) {
        p->image = QImage(w, h, QImage::Format_RGB888);
    }
    return &p->image;
}

QImage ImageProvider::image()
{
    return p->image;
}

void ImageProvider::release()
{
    p->image = QImage();
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

    image = QImage();
//    queue.clear();
}

ImageProviderPrivate::~ImageProviderPrivate()
{
//    queue.clear();
}
