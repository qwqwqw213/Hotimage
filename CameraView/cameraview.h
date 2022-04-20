#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include "QQuickPaintedItem"
#include "QImage"

class CameraView : public QQuickPaintedItem
{
    Q_OBJECT

public:
    CameraView(QQuickItem *parent = nullptr);
    ~CameraView();

    Q_PROPERTY(bool canRead READ canRead NOTIFY canReadChanged)
    bool canRead();

    void refreshView(QImage &img);
    void paint(QPainter *painter) override;

private:
    QImage m_img;

Q_SIGNALS:
    void refresh();
    void canReadChanged();
};

#endif // CAMERAVIEW_H
