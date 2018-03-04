#include <cassert>
#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <mutex>
#include <chrono>
#include "h264_degrader.hh"

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

void H264_degrader::bgra2yuv422p(uint8_t* input, AVFrame* outputFrame, size_t width, size_t height){
  uint8_t * inData[1] = { input };
  int inLinesize[1] = { 4*width };
  
  sws_scale(bgra2yuv422p_context, inData, inLinesize, 0, height, outputFrame->data, outputFrame->linesize);
}

void H264_degrader::yuv422p2bgra(AVFrame* inputFrame, uint8_t* output, size_t width, size_t height){
  uint8_t * inData[3] = { inputFrame->data[0], inputFrame->data[1], inputFrame->data[2] };
  int inLinesize[3] = { width, width/2, width/2 };
  uint8_t * outputArray[1] = { output };
  int outLinesize[1] = { 4*width };

  sws_scale(yuv422p2bgra_context, inData, inLinesize, 0, height, outputArray, outLinesize);
}

H264_degrader::H264_degrader(size_t _width, size_t _height, size_t _bitrate, size_t quantization) :
    width(_width),
    height(_height),
    bitrate(_bitrate),
    frame_count(0),
    quantization(quantization)
{
    buffer = std::move(std::unique_ptr<uint8_t[]>((uint8_t*)aligned_alloc(32,1<<23)));

    avcodec_register_all();

    encoder_codec = avcodec_find_encoder(codec_id);
    if(encoder_codec == NULL){
        std::cout << "encoder_codec: " << codec_id << " not found!" << "\n";
        throw;
    }
    
    decoder_codec = avcodec_find_decoder(codec_id);
    if(decoder_codec == NULL){
        std::cout << "decoder_codec: " << codec_id << " not found!" << "\n";
        throw;
    }

    encoder_context = avcodec_alloc_context3(encoder_codec);
    if(encoder_context == NULL){
        std::cout << "encoder_context: " << codec_id << " not found!" << "\n";
        throw;
    }

    decoder_context = avcodec_alloc_context3(decoder_codec);
    if(decoder_context == NULL){
        std::cout << "decoder_context: " << codec_id << " not found!" << "\n";
        throw;
    }

    // encoder context parameter
    encoder_context->pix_fmt = pix_fmt;
    encoder_context->width = width;
    encoder_context->height = height;

    encoder_context->bit_rate = bitrate;
    encoder_context->bit_rate_tolerance = 1000000;

    encoder_context->time_base = (AVRational){1, 20};
    encoder_context->framerate = (AVRational){60, 1};
    encoder_context->gop_size = 0;
    encoder_context->max_b_frames = 0;
    encoder_context->qmin = quantization;
    encoder_context->qmax = quantization;
    encoder_context->qcompress = 0.5;
    av_opt_set(encoder_context->priv_data, "tune", "zerolatency", 0); // forces no frame buffer delay (https://stackoverflow.com/questions/10155099/c-ffmpeg-h264-creating-zero-delay-stream)
    av_opt_set(encoder_context->priv_data, "preset", "fast", 0);

    // decoder context parameter
    decoder_context->pix_fmt = pix_fmt;
    decoder_context->width = width;
    decoder_context->height = height;

    decoder_context->bit_rate = encoder_context->bit_rate;
    encoder_context->bit_rate_tolerance = encoder_context->bit_rate_tolerance;

    decoder_context->time_base = encoder_context->time_base;
    decoder_context->framerate = encoder_context->framerate;
    decoder_context->gop_size = encoder_context->gop_size;
    decoder_context->max_b_frames = encoder_context->max_b_frames;
    decoder_context->qmin = encoder_context->qmin;
    decoder_context->qmax = encoder_context->qmax;
    decoder_context->qcompress = encoder_context->qcompress;
    av_opt_set(decoder_context->priv_data, "preset", "fast", 0);

    if(avcodec_open2(encoder_context, encoder_codec, NULL) < 0){
        std::cout << "could not open encoder" << "\n";;
        throw;
    }
    
    if(avcodec_open2(decoder_context, decoder_codec, NULL) < 0){
        std::cout << "could not open decoder" << "\n";;
        throw;
    }

    decoder_parser = av_parser_init(decoder_codec->id);
    if(decoder_parser == NULL){
        std::cout << "Decoder parser could not be initialized" << "\n";
        throw;
    }

    encoder_frame = av_frame_alloc();
    if(encoder_frame == NULL) {
        std::cout << "AVFrame not allocated: encoder" << "\n";
        throw;
    }

    decoder_frame = av_frame_alloc();
    if(decoder_frame == NULL) {
        std::cout << "AVFrame not allocated: decoder" << "\n";
        throw;
    }
    
    encoder_frame->width = width;
    encoder_frame->height = height;
    encoder_frame->format = pix_fmt;
    encoder_frame->pts = 0;

    decoder_frame->width = width;
    decoder_frame->height = height;
    decoder_frame->format = pix_fmt;
    decoder_frame->pts = 0;

    if(av_frame_get_buffer(encoder_frame, 32) < 0){
        std::cout << "AVFrame could not allocate buffer: encoder" << "\n";
        throw;
    }

    if(av_frame_get_buffer(decoder_frame, 32) < 0){
        std::cout << "AVFrame could not allocate buffer: decoder" << "\n";
        throw;
    }

    encoder_packet = av_packet_alloc();
    if(encoder_packet == NULL) {
        std::cout << "AVPacket not allocated: encoder" << "\n";
        throw;
    }

    decoder_packet = av_packet_alloc();
    if(decoder_packet == NULL) {
        std::cout << "AVPacket not allocated: decoder" << "\n";
        throw;
    }

  bgra2yuv422p_context = sws_getContext(width, height,
				    AV_PIX_FMT_BGRA, width, height,
				    AV_PIX_FMT_YUV422P, 0, 0, 0, 0);
  if (bgra2yuv422p_context == NULL) {
    std::cout << "BGRA to YUV422P context not found\n";
    throw;
  }


  yuv422p2bgra_context = sws_getContext(width, height,
			    AV_PIX_FMT_YUV422P, width, height, 
			    AV_PIX_FMT_BGRA, 0, 0, 0, 0);

  if (yuv422p2bgra_context == NULL) {
    std::cout << "BGRA to YUV422P context not found\n";
    throw;
  }

}

H264_degrader::~H264_degrader(){
    std::lock_guard<std::mutex> guard(degrader_mutex);

    av_parser_close(decoder_parser);

    avcodec_free_context(&decoder_context);
    avcodec_free_context(&encoder_context);

    av_frame_free(&decoder_frame);
    av_frame_free(&encoder_frame);

    av_packet_free(&decoder_packet);
    av_packet_free(&encoder_packet);

    sws_freeContext(bgra2yuv422p_context);
    sws_freeContext(yuv422p2bgra_context);
}

void H264_degrader::degrade(AVFrame *inputFrame, AVFrame *outputFrame){
    bool output_set = false;

    if(av_frame_make_writable(inputFrame) < 0){
        std::cout << "Could not make the frame writable" << "\n";
        throw;
    }

    // encode frame
    auto encode1 = std::chrono::high_resolution_clock::now();
    inputFrame->pts = frame_count;
    int ret = avcodec_send_frame(encoder_context, inputFrame);
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
            std::cout << "error! multiple parsing passing!" << "\n";
            throw;
        }
        count += 1;
    }
    av_packet_unref(encoder_packet);
    auto encode2 = std::chrono::high_resolution_clock::now();
    auto encodetime = std::chrono::duration_cast<std::chrono::duration<double>>(encode2 - encode1);
    std::cout << "encodetime " << encodetime.count() << "\n";


    // decode frame
    uint8_t *data = buffer.get();
    int data_size = buffer_size;
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
        
        if(ret1 < 0){
            std::cout << "error while parsing the buffer: decoding" << "\n";
            throw;
        }

        data += ret1;
        data_size -= ret1;
        auto parse2 = std::chrono::high_resolution_clock::now();
        auto parsetime = std::chrono::duration_cast<std::chrono::duration<double>>(parse2 - parse1);
        std::cout << "parsetime " << parsetime.count() << "\n";

        auto decode1 = std::chrono::high_resolution_clock::now();
        if(decoder_packet->size > 0){
            if(avcodec_send_packet(decoder_context, decoder_packet) < 0){
                std::cout << "error while decoding the buffer: send_packet" << "\n";
                throw;                
            }

            size_t ret2 = avcodec_receive_frame(decoder_context, outputFrame);
            if (ret2 == AVERROR(EAGAIN) || ret2 == AVERROR_EOF){
                continue;
            }
            else if (ret2 < 0) {
                std::cout << "error during decoding: receive frame" << "\n";
                throw;
            }
            
        }
        output_set = true;
        auto decode2 = std::chrono::high_resolution_clock::now();
        auto decodetime = std::chrono::duration_cast<std::chrono::duration<double>>(decode2 - decode1);
        std::cout << "decodetime " << decodetime.count() << "\n";
    }
    //av_packet_unref(decoder_packet);

    if(!output_set){
        std::memset(outputFrame->data[0], 255, width*height);
        std::memset(outputFrame->data[1], 128, width*height/2);
        std::memset(outputFrame->data[2], 128, width*height/2);
    }

    frame_count += 1;
}

