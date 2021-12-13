#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "QQuickPaintedItem"
#include "QImage"

#include "VideoProcess/videoprocess.h"

class ImagePaintView : public QQuickPaintedItem
{
    Q_OBJECT
public:
    ImagePaintView(QQuickItem *parent = nullptr);
    ~ImagePaintView();

    void paint(QPainter *painter) override;

    Q_INVOKABLE void openStream(const QString &path);
    Q_INVOKABLE void closeStream();

private:
    QImage m_image;
    VideoProcess *decode;

public Q_SLOTS:
    void updateImage(QImage image);
};

#endif // IMAGEVIEW_H
