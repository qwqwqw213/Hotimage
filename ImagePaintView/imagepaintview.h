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

    Q_INVOKABLE void openStream(const QString &path, const int &w, const int &h);
    Q_INVOKABLE void closeStream();

    Q_PROPERTY(bool playing READ playing NOTIFY playStatusChanged)
    bool playing();

private:
    QImage m_image;
    VideoProcess *decode;
    qreal m_x;
    qreal m_y;
    int m_w;
    int m_h;

Q_SIGNALS:
    void playStatusChanged();

public Q_SLOTS:
    void updateImage(QImage image);
};

#endif // IMAGEVIEW_H
