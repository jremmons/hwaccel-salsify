#include <chrono>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

#include "h264_degrader.hh"

#define PIX(x) (x < 0 ? 0 : (x > 255 ? 255 : x))

int main(int argc, char **argv)
{
    if(argc != 4){
        std::cout << "usage: " << argv[0] << " <quantizer (1-64; lower is better)> <input.raw> <ouptut.raw>\n";
        return 0;
    }

    const int quantizer = std::stoi(argv[1]);
    const std::string input_filename = argv[2];
    const std::string output_filename = argv[3];

    std::cout << "input: " << input_filename << "\n";
    std::cout << "ouput: " << output_filename << "\n";    

    const size_t width = 1280;
    const size_t height = 720;
    const size_t bytes_per_pixel = 4;
    const size_t frame_size = width*height*bytes_per_pixel;

    std::ifstream infile(input_filename, std::ios::binary);
    if(!infile.is_open()){
        std::cout << "Could not open file: " << input_filename << "\n";
        return 0;
    }
    
    std::ofstream outfile(output_filename, std::ios::binary);
    if(!outfile.is_open()){
        std::cout << "Could not open file: " << input_filename << "\n";
        return 0;
    }

    std::shared_ptr<uint8_t> input_buffer(new uint8_t[frame_size]);
    std::shared_ptr<uint8_t> output_buffer(new uint8_t[frame_size]);

    std::shared_ptr<uint8_t> y_buffer1(new uint8_t[frame_size/4]);
    std::shared_ptr<uint8_t> u_buffer1(new uint8_t[frame_size/8]);
    std::shared_ptr<uint8_t> v_buffer1(new uint8_t[frame_size/8]);
    std::shared_ptr<uint8_t> y_buffer2(new uint8_t[frame_size/4]);
    std::shared_ptr<uint8_t> u_buffer2(new uint8_t[frame_size/8]);
    std::shared_ptr<uint8_t> v_buffer2(new uint8_t[frame_size/8]);
    
    uint8_t *yuv_input[] = {y_buffer1.get(), u_buffer1.get(), v_buffer1.get()};
    uint8_t *yuv_output[] = {y_buffer2.get(), u_buffer2.get(), v_buffer2.get()};

    H264_degrader degrader(width, height, quantizer);

    while(!infile.eof()){
        infile.read((char*)input_buffer.get(), frame_size);
        
        // convert to yuv420p
        auto bgra2yuv_t1 = std::chrono::high_resolution_clock::now();
        degrader.bgra2yuv420p(input_buffer.get(), degrader.encoder_frame, width, height);
        auto bgra2yuv_t2 = std::chrono::high_resolution_clock::now();
        auto bgra2yuv_time = std::chrono::duration_cast<std::chrono::duration<double>>(bgra2yuv_t2 - bgra2yuv_t1);
        //std::cout << "bgra2yuv_time " << bgra2yuv_time.count() << "\n";
        
        // degrade
        auto degrade_t1 = std::chrono::high_resolution_clock::now();
        degrader.degrade(degrader.encoder_frame, degrader.decoder_frame);
        auto degrade_t2 = std::chrono::high_resolution_clock::now(); 
        auto degrade_time = std::chrono::duration_cast<std::chrono::duration<double>>(degrade_t2 - degrade_t1);
        //std::cout << degrade_time.count() << "\n";

        // convert to bgra and output
        auto yuv2bgra_t1 = std::chrono::high_resolution_clock::now();
        degrader.yuv420p2bgra(degrader.decoder_frame, output_buffer.get(), width, height);
        auto yuv2bgra_t2 = std::chrono::high_resolution_clock::now();
        auto yuv2bgra_time = std::chrono::duration_cast<std::chrono::duration<double>>(yuv2bgra_t2 - yuv2bgra_t1);
        //std::cout << "yuv2bgra_time " << yuv2bgra_time.count() << "\n";

        outfile.write((char*)output_buffer.get(), frame_size);
    }

    return 0;
}
