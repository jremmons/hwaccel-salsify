#pragma once
//#ifndef __H264_DEGRADER_HH__
//#define __H264_DEGRADER_HH__

extern "C" {
#include <libswscale/swscale.h>
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
}

#include <mutex>

class H264_encoder{
public:    
    H264_encoder(size_t _width, size_t _height, size_t quantization);
    ~H264_encoder();

    size_t encode(uint8_t *input, uint8_t *output);

private:
    const AVCodecID codec_id = AV_CODEC_ID_H264;
    const AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;

    const size_t width;
    const size_t height;
    const size_t quantization;    
    size_t frame_count;

    AVCodec *encoder_codec;
    AVCodecContext *encoder_context;
    AVPacket *encoder_packet;
    AVFrame *encoder_frame;

};

//#endif
