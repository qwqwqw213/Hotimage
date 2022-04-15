#include "uvcamera.h"

#include "androidUSBImp.hpp"
#include "Function/pixeloperations.hpp"

#include <QDebug>
#include <QThread>

class UVCameraPrivate
{
public:
    explicit UVCameraPrivate(UVCamera *parent);
    ~UVCameraPrivate();

    UvcHandler *hander;
    QThread *thread;

    bool exit_thread;
    int fd;
    int width;
    int height;
    int fps;
    FrameFormat format;
    FrameFormat formatConvert(const CameraPixelFormat &f) {
        switch (f) {
        case __pix_yuyv: { return UVC_FRAME_FORMAT_YUYV; }
        case __pix_mjpeg: { return UVC_FRAME_FORMAT_MJPEG; }
        case __pix_rgb24: { return UVC_FRAME_FORMAT_RGB; }
        default: { return UVC_FRAME_FORMAT_UNKNOWN; }
        }
    }

    PixelOperations pixel;
    frame_callback callback_function;


private:
    UVCamera *f;
};

UVCamera::UVCamera(QObject *parent)
    : ProviderCamera(parent)
    , p(new UVCameraPrivate(this))
{

}

UVCamera::~UVCamera()
{

}

bool UVCamera::isOpen()
{
    return !p->exit_thread;
}

void UVCamera::open(const int &_fd, const int &_w, const int &_h,
          frame_callback func,
          const int &_fps, const CameraPixelFormat &_format)
{
    if( isOpen() ) {
        return;
    }
    p->fd = _fd;
    p->width = _w;
    p->height = _h;
    if( func ) {
        p->callback_function = func;
    }
    p->fps = _fps;
    p->format = p->formatConvert(_format);
    p->thread->start();
}

void UVCamera::close()
{
    if( !isOpen() ) {
        return;
    }
    p->exit_thread = true;
    emit updateCameraState();
    p->thread->quit();
    p->thread->wait();
    qDebug() << "close uvc, fd:" << p->fd;
}

int UVCamera::currentFd()
{
    return p->fd;
}

int UVCamera::width()
{
    return p->width;
}

int UVCamera::height()
{
    return p->height;
}

UVCameraPrivate::UVCameraPrivate(UVCamera *parent)
{
    f = parent;

    hander = nullptr;
    callback_function = nullptr;

    exit_thread = true;
    thread = new QThread;
    QObject::connect(thread, &QThread::started, [=](){
        hander = new UvcHandler(fd);
        int ret = hander->UvcSetProperty(width, height, fps, format);
        qDebug() << "UvcSetProperty ret:" << ret << width << height << fps << format;
        if( ret == UVC_SUCCESS )
        {
            ret = hander->UvcRequestStart();
            if( ret == UVC_SUCCESS )
            {
                exit_thread = false;
                f->updateCameraState();
                while (!exit_thread) {
                    std::tuple<Uvcframe*, int> t = hander->UvcGetFrame(100 * 1000);
                    Uvcframe *data = std::get<0>(t);
                    int err = std::get<1>(t);
                    if( data == NULL ) {
                        qDebug() << "uvc invaild data" << err << fd;
                        QThread::msleep(1000);
                        continue;
                    }
                    if( data->data_bytes < (data->width * data->height * 2) ) {
                        continue;
                    }

                    if( callback_function ) {
                        callback_function(f, data->data, data->width, data->height, data->data_bytes);
                    }
                    else {
                        switch (data->frame_format) {
                        case UVC_FRAME_FORMAT_YUYV: {
                            QImage *rgb = f->rgb();
                            YUV *yuv = reinterpret_cast<YUV *>(data->data);
                            pixel.yuv422_to_rgb(yuv, rgb->bits(), data->width, data->height);

                            QImage img = rgb->copy();
                            f->setUrlImage(img);
                        }
                        default: break;
                        }
                    }
                }
            }
            else {
                qDebug() << QString("ERROR(code: %1): UvcRequestStart fail").arg(ret);

            }
        }
        else {
            qDebug() << QString("ERROR(code: %1): UvcSetProperty fail").arg(ret);

        }
    });
    QObject::connect(thread, &QThread::finished, [=](){
        if( hander->result() >= 0 ) {
            hander->UvcRequestStop();
        }
        delete hander;
        hander = nullptr;
    });
}

UVCameraPrivate::~UVCameraPrivate()
{

}
