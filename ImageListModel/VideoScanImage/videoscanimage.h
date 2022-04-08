#ifndef VIDEOSCANIMAGE_H
#define VIDEOSCANIMAGE_H

#include "QQuickImageProvider"

class VideoScanImagePrivate;
class VideoScanImage : public QQuickImageProvider
{
public:
    VideoScanImage(const QString &url);
    ~VideoScanImage();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    QString url();
    QString addQueue(const QString &path, QString &videoTotalTime);

private:
    friend class VideoScanImagePrivate;
    QScopedPointer<VideoScanImagePrivate> p;
};

#endif // VIDEOSCANIMAGE_H
