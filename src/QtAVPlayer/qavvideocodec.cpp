/*********************************************************
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com> *
 *                                                       *
 * This file is part of QtAVPlayer.                      *
 * Free Qt Media Player based on FFmpeg.                 *
 *********************************************************/

#include "qavvideocodec_p.h"
#include "qavhwdevice_p.h"
#include "qavcodec_p_p.h"
#include "qavpacket_p.h"
#include "qavframe.h"
#include "qavvideoframe.h"
#include <QDebug>

extern "C" {
#include <libavutil/pixdesc.h>
#include <libavcodec/avcodec.h>
}

QT_BEGIN_NAMESPACE

class QAVVideoCodecPrivate : public QAVCodecPrivate
{
public:
    QScopedPointer<QAVHWDevice> hw_device;
};

static bool isSoftwarePixelFormat(AVPixelFormat from)
{
    switch (from) {
    case AV_PIX_FMT_VAAPI:
    case AV_PIX_FMT_VDPAU:
    case AV_PIX_FMT_MEDIACODEC:
    case AV_PIX_FMT_VIDEOTOOLBOX:
    case AV_PIX_FMT_D3D11:
    case AV_PIX_FMT_D3D11VA_VLD:
    case AV_PIX_FMT_OPENCL:
    case AV_PIX_FMT_CUDA:
    case AV_PIX_FMT_DXVA2_VLD:
#if FF_API_XVMC
    case AV_PIX_FMT_XVMC:
#endif
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(58, 134, 0)
    case AV_PIX_FMT_VULKAN:
#endif
    case AV_PIX_FMT_DRM_PRIME:
    case AV_PIX_FMT_MMAL:
    case AV_PIX_FMT_QSV:
        return false;
    default:
        return true;
    }
}

static AVPixelFormat negotiate_pixel_format(AVCodecContext *c, const AVPixelFormat *f)
{
    auto d = reinterpret_cast<QAVVideoCodecPrivate *>(c->opaque);

    QList<AVPixelFormat> softwareFormats;
    QList<AVPixelFormat> hardwareFormats;
    for (int i = 0; f[i] != AV_PIX_FMT_NONE; ++i) {
        if (!isSoftwarePixelFormat(f[i])) {
            hardwareFormats.append(f[i]);
            continue;
        }
        softwareFormats.append(f[i]);
    }

    qDebug() << "Available pixel formats:";
    for (auto a : softwareFormats) {
        auto dsc = av_pix_fmt_desc_get(a);
        qDebug() << "  " << dsc->name << ": AVPixelFormat(" << a << ")";
    }

    for (auto a : hardwareFormats) {
        auto dsc = av_pix_fmt_desc_get(a);
        qDebug() << "  " << dsc->name << ": AVPixelFormat(" << a << ")";
    }

    AVPixelFormat pf = !softwareFormats.isEmpty() ? softwareFormats[0] : AV_PIX_FMT_NONE;
    const char *decStr = "software";
    if (d->hw_device) {
        for (auto f : hardwareFormats) {
            if (f == d->hw_device->format()) {
                pf = d->hw_device->format();
                decStr = "hardware";
                break;
            }
        }
    }

    auto dsc = av_pix_fmt_desc_get(pf);
    if (dsc)
        qDebug() << "Using" << decStr << "decoding in" << dsc->name;

    return pf;
}

QAVVideoCodec::QAVVideoCodec(QObject *parent)
    : QAVFrameCodec(*new QAVVideoCodecPrivate, parent)
{
    d_ptr->avctx->opaque = d_ptr.data();
    d_ptr->avctx->get_format = negotiate_pixel_format;
}

void QAVVideoCodec::setDevice(QAVHWDevice *d)
{
    d_func()->hw_device.reset(d);
}

QAVHWDevice *QAVVideoCodec::device() const
{
    return d_func()->hw_device.data();
}

QT_END_NAMESPACE
