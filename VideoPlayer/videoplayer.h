#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "QQuickImageProvider"
#include "QQuickPaintedItem"
#include "QImage"
#include "QQueue"
#include "QMutex"

#include "VideoProcess/videoprocess.h"

class VideoProvider;

class VideoPlayer : public QQuickPaintedItem
{
    Q_OBJECT
public:
    VideoPlayer(QQuickItem *parent = nullptr);
    ~VideoPlayer();

    void paint(QPainter *painter) override;

    Q_INVOKABLE void openStream(const QString &path, const int &w, const int &h, const int &index);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void closeStream();

    Q_PROPERTY(int playing READ playing NOTIFY playStatusChanged)
    int playing();
    Q_PROPERTY(int playIndex READ playIndex NOTIFY playStatusChanged)
    int playIndex();

    Q_PROPERTY(int imageWidth READ imageWidth NOTIFY imageSizeChanged)
    int imageWidth();
    Q_PROPERTY(int imageHeight READ imageHeight NOTIFY imageSizeChanged)
    int imageHeight();

    Q_PROPERTY(QString currentTime READ currentTime NOTIFY frameUpdate)
    QString currentTime();
    Q_PROPERTY(QString totalTime READ totalTime NOTIFY frameUpdate)
    QString totalTime();
    Q_PROPERTY(qreal progress READ progress NOTIFY frameUpdate)
    qreal progress();
    Q_PROPERTY(QString frameUrl READ frameUrl NOTIFY frameUpdate)
    QString frameUrl();
    VideoProvider *provider();
    QString providerUrl();


private:
    QImage m_image;
    VideoProcess *decode;
    qreal m_x;
    qreal m_y;
    int m_w;
    int m_h;
    int m_index;
    VideoProvider *videoProvider;

Q_SIGNALS:
    void playStatusChanged();
    void imageSizeChanged();
    void currentTimeChanged();
    void frameUpdate();

public Q_SLOTS:
    void updateImage(QImage image);
};

class VideoProvider : public QQuickImageProvider
{
public:
    VideoProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {
        frame = 0;
    }
    ~VideoProvider() {

    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        return QPixmap();
    }
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        QImage img;
        if( queue.size() > 0 ) {
            mutex.lock();
            img = queue.dequeue();
            mutex.unlock();
        }
        return img;
    }

    QString url() {
        return QString("videostream");
    }
    QString qmlUrl() {
        frame ++;
        return QString("image://videostream/%1").arg(frame);
    }

    void add(QImage img) {
        mutex.lock();
        queue.enqueue(img);
        mutex.unlock();
    }

    void release() {
        frame = 0;
        mutex.lock();
        queue.clear();
        mutex.unlock();
    }

private:
    QQueue<QImage> queue;
    QMutex mutex;
    quint64 frame;
};

#endif // VIDEOPLAYER_H
