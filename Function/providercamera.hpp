#ifndef PROVIDERCAMERA
#define PROVIDERCAMERA

#include "QObject"
#include "ImageProvider/imageprovider.h"

typedef void (*frame_callback)(void *content, void *data, const int &width, const int &height, const int &bufferLength);

enum CameraPixelFormat
{
    __pix_invalid = 0,
    __pix_mjpeg,
    __pix_yuyv,
    __pix_yuv420p,
    __pix_rgb24,
};

class ProviderCamera : public QObject
{
    Q_OBJECT
public:
    ProviderCamera(QObject *parent = nullptr)
        : QObject(parent) {
        provider = nullptr;
        rgbImage = QImage();
    }
    ~ProviderCamera() { rgbImage = QImage(); }

    static int byteSize(const int &w, const int &h, const CameraPixelFormat &format) {
        switch (format) {
        case CameraPixelFormat::__pix_yuyv: { return (w * h * 2); }
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

    QImage *rgb() {
        if( rgbImage.isNull() ) {
            rgbImage = QImage(width(), height(), QImage::Format_RGB888);
        }
        return &rgbImage;
    }

    // qml 加载图片部分
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
    void setUrlImage(QImage &image) { provider->append(image); }

Q_SIGNALS:
    void updateCameraState();
    void updateImageFrameUrl();

private:
    ImageProvider *provider;
    QImage rgbImage;
};

#endif
