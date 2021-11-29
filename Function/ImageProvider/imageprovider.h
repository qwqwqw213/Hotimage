#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include "QQuickImageProvider"

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider(const QString &url);
    ~ImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    void setEmptyRgbImage(const int &w, const int &h);
    void setImage(QImage image);

    uint8_t * data();

    QImage &image();

    QString url();
    QString qmlUrl();

    void release();

private:
    QImage m_image;
    quint8 m_requesTime;
    QString m_url;
    QString m_qmlUrl;
};

#endif // IMAGEPROVIDER_H
