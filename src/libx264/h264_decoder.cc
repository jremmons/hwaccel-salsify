#include <cassert>
#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <mutex>
#include <chrono>
#include "h264_decoder.hh"

extern "C" {
#include <stdlib.h>
#include <libswscale/swscale.h>
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/common.h"
#include "libavutil/mathematics.h"
}

H264_decoder::H264_decoder(size_t _width, size_t _height) :
    width(_width),
    height(_height),
    frame_count(0),
    decoder_codec(NULL),
    decoder_context(NULL),
    decoder_parser(NULL),
    decoder_frame(NULL),
    decoder_packet(NULL)
{
    avcodec_register_all();

    decoder_codec = avcodec_find_decoder(codec_id);
    if(decoder_codec == NULL){
        std::cout << "decoder_codec: " << codec_id << " not found!" << "\n";
        throw;
    }

    decoder_context = avcodec_alloc_context3(decoder_codec);
    if(decoder_context == NULL){
        std::cout << "decoder_context: " << codec_id << " not found!" << "\n";
        throw;
    }

    // decoder context parameter
    decoder_context->pix_fmt = pix_fmt;
    decoder_context->width = width;
    decoder_context->height = height;

    decoder_context->bit_rate = 1<<10;
    decoder_context->bit_rate_tolerance = 0;

    decoder_context->time_base = (AVRational){1, 20};
    decoder_context->framerate = (AVRational){60, 1};
    decoder_context->gop_size = 0;
    decoder_context->max_b_frames = 0;
    decoder_context->qmin = 0;
    decoder_context->qmax = 0;
    decoder_context->qcompress = 0.5;
    av_opt_set(decoder_context->priv_data, "tune", "zerolatency", 0); // forces no frame buffer delay (https://stackoverflow.com/questions/10155099/c-ffmpeg-h264-creating-zero-delay-stream)

    if(avcodec_open2(decoder_context, decoder_codec, NULL) < 0){
        std::cout << "could not open decoder" << "\n";;
        throw;
    }

    decoder_parser = av_parser_init(decoder_codec->id);
    if(decoder_parser == NULL){
        std::cout << "Decoder parser could not be initialized" << "\n";
        throw;
    }

    decoder_frame = av_frame_alloc();
    if(decoder_frame == NULL) {
        std::cout << "AVFrame not allocated: decoder" << "\n";
        throw;
    }
    
    decoder_frame->width = width;
    decoder_frame->height = height;
    decoder_frame->format = pix_fmt;
    decoder_frame->pts = 0;

    if(av_frame_get_buffer(decoder_frame, 32) < 0){
        std::cout << "AVFrame could not allocate buffer: decoder" << "\n";
        throw;
    }

    decoder_packet = av_packet_alloc();
    if(decoder_packet == NULL) {
        std::cout << "AVPacket not allocated: decoder" << "\n";
        throw;
    }

}


H264_decoder::~H264_decoder(){
    av_parser_close(decoder_parser);
    avcodec_free_context(&decoder_context);
    av_frame_free(&decoder_frame);
    av_packet_free(&decoder_packet);
}


void H264_decoder::decode(const std::shared_ptr<uint8_t[]> yuv420_compressed_input, const size_t yuv420_compressed_input_len, std::shared_ptr<uint8_t[]> yuv420_raw_output){

    uint8_t *data = yuv420_compressed_input.get();
    int data_size = yuv420_compressed_input_len;
    bool output_set = false;

    std::cout << "compressed frame size: " << yuv420_compressed_input_len << "\n";
    
    while(data_size > 0){
        auto parse1 = std::chrono::high_resolution_clock::now();
        size_t ret1 = av_parser_parse2(decoder_parser,
                                       decoder_context, 
                                       &decoder_packet->data, 
                                       &decoder_packet->size,
                                       data,
                                       data_size,
                                       AV_NOPTS_VALUE, 
                                       AV_NOPTS_VALUE, 
                                       0);
        
        std::cout << "output_size " << decoder_packet->size << "\n";    

        if(ret1 < 0){
            std::cout << "error while parsing the buffer: decoding" << "\n";
            throw;
        }

        data += ret1;
        data_size -= ret1;

        std::cout << "consumed bytes: " << ret1 << "\n";
        auto parse2 = std::chrono::high_resolution_clock::now();
        auto parsetime = std::chrono::duration_cast<std::chrono::duration<double>>(parse2 - parse1);
        //std::cout << "parsetime " << parsetime.count() << "\n";

        auto decode1 = std::chrono::high_resolution_clock::now();
        if(decoder_packet->size > 0){
            if(avcodec_send_packet(decoder_context, decoder_packet) < 0){
                std::cout << "error while decoding the buffer: send_packet" << "\n";
                throw;                
            }

            size_t ret2 = avcodec_receive_frame(decoder_context, decoder_frame);
            if (ret2 == AVERROR(EAGAIN) || ret2 == AVERROR_EOF){
                continue;
            }
            else if (ret2 < 0) {
                std::cout << "error during decoding: receive frame" << "\n";
                throw;
            }
            output_set = true;
        }
        auto decode2 = std::chrono::high_resolution_clock::now();
        auto decodetime = std::chrono::duration_cast<std::chrono::duration<double>>(decode2 - decode1);
        //std::cout << "decodetime " << decodetime.count() << "\n";
    }
    //av_packet_unref(decoder_packet);

    std::cout << "output_set " << output_set << "\n";
    if(!output_set){
        std::cout << "output was not set\n";
        std::memset(yuv420_raw_output.get(), 255, width*height);
        std::memset(yuv420_raw_output.get() + width*height, 128, width*height/4);
        std::memset(yuv420_raw_output.get() + width*height + width*height/2, 128, width*height/4);
    }
    else{
        std::memcpy(yuv420_raw_output.get(), decoder_frame->data[0], width*height);
        std::memcpy(yuv420_raw_output.get() + width*height, decoder_frame->data[1], width*height/4);
        std::memcpy(yuv420_raw_output.get() + width*height + width*height/4, decoder_frame->data[2], width*height/4);
    }
    
}

