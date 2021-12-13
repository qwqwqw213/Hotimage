#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include "QQuickImageProvider"

class ImageProviderPrivate;
class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider(const QString &url);
    ~ImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    void addQueue(QImage image);
    void clear();

    QString url();
    QString qmlUrl();

private:
    friend class ImageProviderPrivate;
    QScopedPointer<ImageProviderPrivate> p;
};

#endif // IMAGEPROVIDER_H
