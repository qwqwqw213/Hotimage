#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "QQuickPaintedItem"
#include "QImage"

#include "TcpCamera/tcpcamera.h"

class ImageView : public QQuickPaintedItem
{
    Q_OBJECT
public:
    ImageView(QQuickItem *parent = nullptr);
    ~ImageView();

    Q_PROPERTY(TcpCamera *camera READ camera WRITE setCamera)
    TcpCamera *camera();
    void setCamera(TcpCamera *camera);

    void paint(QPainter *painter) override;

private:
    QImage m_image;
    TcpCamera *m_camera;
};

#endif // IMAGEVIEW_H
