#ifndef ANDROIDUSBIMP_h
#define ANDROIDUSBIMP_h

#include "libusb.h"
#include "libuvc.h"
#include <string>
#include <algorithm>

typedef uvc_frame_t Uvcframe;
typedef uvc_frame_callback_t UvcCallBack;
typedef uvc_frame_format FrameFormat;

class UvcHandler
{
public:
    inline UvcHandler(int fd_from_android)
    {
        androidPFD = fd_from_android;
        //
        libusb_set_option(NULL, LIBUSB_OPTION_NO_DEVICE_DISCOVERY, NULL);

        uvc_init(&libuvcCTX, NULL);

        libuvcRes = uvc_wrap(androidPFD, libuvcCTX, &libuvcDeviceHandle);
        if (libuvcRes != 0)
            throw(int) libuvcRes;
        //
    };

    inline int UvcSetProperty(int width, int height, int frameRate, FrameFormat format)
    {
        libuvcRes = uvc_get_stream_ctrl_format_size(
            libuvcDeviceHandle, &libuvcCtrl,
            format, width, height, frameRate);

        return libuvcRes;
    }

    inline int UvcRequestStart()
    {
        uvc_stream_open_ctrl(libuvcDeviceHandle, &libuvcStreamCtl, &libuvcCtrl);
        return uvc_stream_start(libuvcStreamCtl ,NULL, NULL, 0);
    }

    inline void UvcRequestStop()
    {
        uvc_stream_close(libuvcStreamCtl);
    }

    // -1 for unblock
    inline std::tuple<Uvcframe*,int> UvcGetFrame(int timeout_in_us)
    {
        int error = uvc_stream_get_frame(libuvcStreamCtl, &framePointer, timeout_in_us);
        return std::make_tuple(framePointer,error);
    }

    inline uvc_error_t result() {
        return libuvcRes;
    }

    inline ~UvcHandler()
    {
        if( libuvcRes >= 0 ) {
            uvc_stream_close(libuvcStreamCtl);
        }
    }

private:
    int androidPFD;
    //
    libusb_context *libusbCTX;
    libusb_device_handle *libusbHandle;
    //
    Uvcframe *framePointer;
    uvc_context_t *libuvcCTX;
    uvc_device_t *libuvcDevice;
    uvc_device_handle_t *libuvcDeviceHandle;
    uvc_stream_ctrl_t libuvcCtrl;
    uvc_error_t libuvcRes;

    uvc_stream_handle *libuvcStreamCtl;
    //
};

#endif
