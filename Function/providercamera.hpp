#ifndef PROVIDERCAMERA
#define PROVIDERCAMERA

#include "QObject"
#include "QSettings"
#include "QGuiApplication"
#include "QStandardPaths"
#include "QDebug"

#include "pixeloperations.hpp"
#include "ImageProvider/imageprovider.h"

typedef void (*frame_callback)(void *content, void *data, const int &width, const int &height, const int &bufferLength);

enum CameraPixelFormat {
    __pix_invalid = 0,
    __pix_mjpeg,
    __pix_yuyv,
    __pix_yuv420p,
    __pix_rgb24,
    __pix_custom,
};

enum CameraFlip {
    __no_flip = 0,
    __horizontally_flip,
    __vertically_flip,
    __diagonally_flip,
};

class ProviderCamera : public QObject
{
    Q_OBJECT
public:
    ProviderCamera(QObject *parent = nullptr)
        : QObject(parent) {
        provider = nullptr;
        rgbImage = QImage();

#ifndef Q_OS_WIN32
        QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString("/infrared.ini");
#else
        QString path = QGuiApplication::applicationDirPath() + QString("/provodercamera.ini");
#endif
        settings = new QSettings(path, QSettings::IniFormat);

        currentRotation = settings->value("Normal/Rotation", __no_flip).toInt();
    }
    ~ProviderCamera() {
//        qDebug() << "~ProviderCamera:" << settings->fileName();

        rgbImage = QImage();

        settings->setValue("Normal/Rotation", currentRotation);
        delete settings;
        settings = nullptr;
    }

    static size_t byteSize(const int &w, const int &h, const CameraPixelFormat &format) {
        switch (format) {
        case CameraPixelFormat::__pix_yuyv: { return (w * h * 2); }
        case CameraPixelFormat::__pix_yuv420p: { return (w * h * 3 / 2); }
        case CameraPixelFormat::__pix_rgb24: { return (w * h * 3); }
        default: return 0;
        }
    }

    static int bytesPerLine(const int &w, const int &h, const CameraPixelFormat &format) {
        int size = byteSize(w, h, format);
        return (size / h);
    }

    Q_PROPERTY(bool isOpen READ isOpen NOTIFY updateCameraState)
    virtual bool isOpen() { return false; }

    virtual int width() { return 0; }
    virtual int height() { return 0; }
    virtual CameraPixelFormat pixelFormat() { return __pix_invalid; }
    virtual int fps() { return 0; }
    int cx() { return (width() / 2); }
    int cy() { return (height() / 2); }

    QImage *rgb() {
        if( rgbImage.isNull() ) {
            rgbImage = QImage(width(), height(), QImage::Format_RGB888);
        }
        return &rgbImage;
    }

    Q_PROPERTY(int rotationIndex READ rotationIndex WRITE setRotationIndex NOTIFY updateRotationIndex)
    int rotationIndex() { return currentRotation; }
    void setRotationIndex(const int &index) {
        currentRotation = index;
        emit updateRotationIndex();
    }
    Q_PROPERTY(int rotationCount READ rotationCount CONSTANT)
    int rotationCount() { return 4; }
    Q_INVOKABLE QString rotation(const int & index) {
        switch (index) {
        case __no_flip: { return tr("No flip"); }
        case __horizontally_flip: { return tr("Horizontally"); }
        case __vertically_flip: { return tr("Vertically"); }
        case __diagonally_flip: { return tr("Diagonally"); }
        }
        return QString("Unknow");
    }

    // qml ??????????????????
    Q_PROPERTY(bool canReadUrl READ canReadUrl NOTIFY updateImageFrameUrl)
    bool canReadUrl() { return provider->canRead(); }
    void setFrameUrl(QQmlApplicationEngine *e, const QString &path) {
        if( provider == nullptr ) {
            provider = new ImageProvider;
            QObject::connect(provider, &ImageProvider::imageEnqueue,
                             this, &ProviderCamera::updateImageFrameUrl);
            provider->setUrl(e, path);
        }
    }
    Q_PROPERTY(QString frameUrl READ frameUrl NOTIFY updateImageFrameUrl)
    QString frameUrl() { return provider->url(); }
    Q_PROPERTY(QString freeze READ freeze CONSTANT)
    QString freeze() { return provider->freezeUrl(); }
    void setUrlImage(QImage &image) { provider->append(image); }
    void clearImage() { provider->clear(); }

    PixelOperations * pixelOperations() {
        if( pixel.isNull() ) {
            pixel.reset(new PixelOperations);
        }
        return pixel.data();
    }

    Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY updateContrast)
    int contrast() { return pixel->contrastLevel(); }
    void setContrast(const int &index) { pixel->setContrastLevel((ContrastLevel)index); emit updateContrast(); }

protected:
    // ?????????????????????????????????????????????
    // @param ????????????
    // @param ??????????????????, 0 or 1
    QPoint rotatePoint(const QPoint &p, const int &start = 0) {
        int x = p.x();
        int y = p.y();
        int minus = (start == 1 ? 0 : 1);
        if( (rotationIndex() & 0x01) == __horizontally_flip ) {
            x = cx() + (cx() - minus - x);
        }
        if( (rotationIndex() & 0x02) == __vertically_flip ) {
            y = cy() + (cy() - minus - y);
        }
        return QPoint(x, y);
    }

    QLine rotateLine(const QLine &l, const int &start = 0) {
        int x0 = l.x1();
        int y0 = l.y1();
        int x1 = l.x2();
        int y1 = l.y2();
        int minus = (start == 1 ? 0 : 1);
        if( (rotationIndex() & 0x01) == __horizontally_flip ) {
            x0 = cx() + (cx() - minus - x0);
            x1 = cx() + (cx() - minus - x1);
        }
        if( (rotationIndex() & 0x02) == __vertically_flip ) {
            y0 = cy() + (cy() - minus - y0);
            y1 = cy() + (cy() - minus - y1);
        }
        return QLine(x0, y0, x1, y1);
    }

    QRect rotateRect(const QRect &r, const int &start = 0) {
        int x = r.x();
        int y = r.y();
        int minus = (start == 1 ? 0 : 1);
        if( (rotationIndex() & 0x01) == __horizontally_flip ) {
            x = cx() + (cx() - minus - x) - r.width();
        }
        if( (rotationIndex() & 0x02) == __vertically_flip ) {
            y = cy() + (cy() - minus - y) - r.height();
        }
        return QRect(x, y, r.width(), r.height());
    }

Q_SIGNALS:
    void updateCameraState();
    void updateImageFrameUrl();
    void updateRotationIndex();
    void updateContrast();

private:
    QScopedPointer<PixelOperations> pixel;
    ImageProvider *provider;
    QImage rgbImage;

    QSettings *settings;
    int currentRotation;
};

#endif
