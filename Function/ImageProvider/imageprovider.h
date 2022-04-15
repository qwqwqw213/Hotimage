#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QQmlApplicationEngine>
#include <QDateTime>

#include "Function/queue.hpp"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class ImageProvider : public QQuickImageProvider
#else
class ImageProvider : public QObject, public QQuickImageProvider
#endif
{
    Q_OBJECT

public:
    explicit ImageProvider(const bool &realtime = true);
    ~ImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize) override;
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize) override;

    void append(QImage &img);

    // qml url构成
    // image://path/index
    // @image://  固定不变
    // @path      图片路径, 不能够重复
    // @index     图片索引, 每次更新需要改变, 不然不会刷新图片
    QString url();
    bool setUrl(QQmlApplicationEngine *e, const QString &path);

    bool hasUrl();

    // 当队列有图片时才读取url
    // 不然输出无图片警告
    bool canRead();

private:
    Queue<QImage> m_queue;
    QString m_url;
    QImage m_image;
    bool m_realtime;

Q_SIGNALS:
    void imageEnqueue();
};

#endif // IMAGEPROVIDER_H
