#ifndef UVCAMERA_H
#define UVCAMERA_H

#include "Function/providercamera.hpp"

class UVCameraPrivate;
class UVCamera : public ProviderCamera
{
    Q_OBJECT

public:
    explicit UVCamera(QObject *parent = nullptr);
    ~UVCamera();

    bool isOpen() override;
    void open(const int &_fd, const CameraPixelFormat &_format = __pix_yuyv,
              frame_callback func = nullptr, void *content = nullptr);
    void close();
    int currentFd();

    int width() override;
    int height() override;

    int zoomAbsolute(const uint16_t &value);

private:
    friend class UVCameraPrivate;
    QScopedPointer<UVCameraPrivate> p;
};

#endif // UVCAMERA_H
