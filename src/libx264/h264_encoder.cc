#include <cassert>
#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <mutex>
#include <chrono>
#include "h264_encoder.hh"

extern "C" {
#include <stdlib.h>
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/common.h"
#include "libavutil/mathematics.h"
}

H264_encoder::H264_encoder(size_t _width, size_t _height, size_t quantization) :
    width(_width),
    height(_height),
    quantization(quantization),
    frame_count(0),
    buffer(NULL),
    encoder_codec(NULL),
    encoder_context(NULL),
    encoder_frame(NULL),
    encoder_packet(NULL)
{
    buffer = std::move(std::unique_ptr<uint8_t[]>((uint8_t*)aligned_alloc(32,1<<23)));

    avcodec_register_all();

    encoder_codec = avcodec_find_encoder(codec_id);
    if(encoder_codec == NULL){
        std::cout << "encoder_codec: " << codec_id << " not found!" << "\n";
        throw;
    }
    
    encoder_context = avcodec_alloc_context3(encoder_codec);
    if(encoder_context == NULL){
        std::cout << "encoder_context: " << codec_id << " not found!" << "\n";
        throw;
    }

    // encoder context parameter
    encoder_context->pix_fmt = pix_fmt;
    encoder_context->width = width;
    encoder_context->height = height;

    encoder_context->bit_rate = 1<<10;
    encoder_context->bit_rate_tolerance = 0;

    encoder_context->time_base = (AVRational){1, 20};
    encoder_context->framerate = (AVRational){60, 1};
    encoder_context->gop_size = 0;
    encoder_context->max_b_frames = 0;
    encoder_context->qmin = quantization;
    encoder_context->qmax = quantization;
    encoder_context->qcompress = 0.5;
    av_opt_set(encoder_context->priv_data, "tune", "zerolatency", 0); // forces no frame buffer delay (https://stackoverflow.com/questions/10155099/c-ffmpeg-h264-creating-zero-delay-stream)
    av_opt_set(encoder_context->priv_data, "preset", "fast", 0);

    if(avcodec_open2(encoder_context, encoder_codec, NULL) < 0){
        std::cout << "could not open encoder" << "\n";;
        throw;
    }
    
    encoder_frame = av_frame_alloc();
    if(encoder_frame == NULL) {
        std::cout << "AVFrame not allocated: encoder" << "\n";
        throw;
    }

    encoder_frame->width = width;
    encoder_frame->height = height;
    encoder_frame->format = pix_fmt;
    encoder_frame->pts = 0;

    if(av_frame_get_buffer(encoder_frame, 32) < 0){
        std::cout << "AVFrame could not allocate buffer: encoder" << "\n";
        throw;
    }

    encoder_packet = av_packet_alloc();
    if(encoder_packet == NULL) {
        std::cout << "AVPacket not allocated: encoder" << "\n";
        throw;
    }

}

H264_encoder::~H264_encoder(){
    avcodec_free_context(&encoder_context);
    av_frame_free(&encoder_frame);
    av_packet_free(&encoder_packet);

}

size_t H264_encoder::encode(const std::shared_ptr<uint8_t[]> yuv420_raw_input, std::shared_ptr<uint8_t[]> yuv420_compressed_output) {

    
    if(av_frame_make_writable(encoder_frame) < 0){
        std::cout << "Could not make the frame writable" << "\n";
        throw;
    }

    assert(encoder_frame->linesize[0] == width);
    assert(encoder_frame->linesize[1] == width / 2);
    assert(encoder_frame->linesize[2] == width / 2);
    
    std::memcpy(encoder_frame->data[0], yuv420_raw_input.get(), width*height);
    std::memcpy(encoder_frame->data[1], yuv420_raw_input.get() + width*height, width*height / 4);
    std::memcpy(encoder_frame->data[2], yuv420_raw_input.get() + width*height + width*height / 4, width*height / 4);
    
    // encode frame
    auto encode1 = std::chrono::high_resolution_clock::now();

    encoder_frame->pts = frame_count;
    frame_count++;
    int ret = avcodec_send_frame(encoder_context, encoder_frame);
    if (ret < 0) {
        std::cout << "error sending a frame for encoding" << "\n";
        throw;
    }

    int buffer_size = 0;
    int count = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(encoder_context, encoder_packet);

        if (ret == AVERROR(EAGAIN)){
            break;
            std::cout << "did I make it here?\n";
        }
        else if(ret == AVERROR_EOF){
            break;
        }
        else if (ret < 0) {
            std::cout << "error during encoding" << "\n";
            throw;
        }

        buffer_size = encoder_packet->size;
        assert(encoder_packet->size + AV_INPUT_BUFFER_PADDING_SIZE >= buffer_size);
        std::memcpy(buffer.get(), encoder_packet->data, encoder_packet->size);
        
        if(count > 0){
            std::cout << "error! multiple parsing passes!" << "\n";
            throw;
        }
        count += 1;
    }
    av_packet_unref(encoder_packet);
    auto encode2 = std::chrono::high_resolution_clock::now();
    auto encodetime = std::chrono::duration_cast<std::chrono::duration<double>>(encode2 - encode1);
    //std::cout << "encodetime " << encodetime.count() << "\n";

    uint8_t *data = buffer.get();
    size_t data_size = buffer_size;

    std::cout << "encoded_frame_size: " << buffer_size << "\n";    
    std::memcpy(yuv420_raw_input.get(), data, data_size);
    
    return data_size;

}

