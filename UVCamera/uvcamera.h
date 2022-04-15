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
    void open(const int &_fd, const int &_w, const int &_h,
              frame_callback func = nullptr,
              const int &_fps = 25, const CameraPixelFormat &_format = __pix_yuyv);
    void close();
    int currentFd();

    int width() override;
    int height() override;

private:
    friend class UVCameraPrivate;
    QScopedPointer<UVCameraPrivate> p;
};

#endif // UVCAMERA_H
