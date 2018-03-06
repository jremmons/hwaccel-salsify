#pragma once

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
}

class H264_decoder{
public:    
    H264_decoder(size_t _width, size_t _height);
    ~H264_decoder();

    void decode(uint8_t *input, size_t len, uint8_t *output);

private:
    const AVCodecID codec_id = AV_CODEC_ID_H264;
    const AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;

    const size_t width;
    const size_t height;
    size_t frame_count;
    
    AVCodec *decoder_codec;
    AVCodecContext *decoder_context;
    AVCodecParserContext *decoder_parser;
    AVFrame *decoder_frame;
    AVPacket *decoder_packet;

};

