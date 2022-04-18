#ifndef ANDROIDUSBIMP_h
#define ANDROIDUSBIMP_h

#include "libusb.h"
#include "libuvc.h"
#include "libuvc_internal.h"
#include <string>
#include <algorithm>

#include "QDebug"

typedef uvc_frame_t Uvcframe;
typedef uvc_frame_callback_t UvcCallBack;
typedef uvc_frame_format FrameFormat;

typedef struct {
    FrameFormat format;
    int width;
    int height;
    int fps;
} CameraParam;

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

        mapFormat = std::map<std::string, FrameFormat> {
            {"YUY2", UVC_FRAME_FORMAT_YUYV},
            {"MJPG", UVC_FRAME_FORMAT_MJPEG}
        };
    };

    inline std::vector<CameraParam> GetCameraParam() {
        std::vector<CameraParam> cameraInfo;

        uvc_streaming_interface_t *stream_ifs;
        DL_FOREACH(libuvcDeviceHandle->info->stream_ifs, stream_ifs) {
            uvc_format_desc_t *format;
            DL_FOREACH(stream_ifs->format_descs, format) {
                uvc_frame_desc_t *frame;
                DL_FOREACH(format->frame_descs, frame) {
                    uint32_t *interval;
                    if (frame->intervals) {
                        for (interval = frame->intervals; *interval; ++interval) {
                            int fps = 10000000 / *interval;
                            FrameFormat f = mapFormat.at(reinterpret_cast<char *>(format->fourccFormat));
                            cameraInfo.push_back({f, frame->wWidth, frame->wHeight, fps});
                        }
                    }
                }
            }
        }
        return cameraInfo;
    }

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

    inline int ZoomAbsolute(const uint16_t &value) {
        return uvc_set_zoom_abs(libuvcDeviceHandle, value);
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

    std::map<std::string, FrameFormat> mapFormat;

    inline FrameFormat toFormat(const uint8_t *f) {

    }
    //
};

#endif
