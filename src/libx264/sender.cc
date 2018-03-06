#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "h264_encoder.hh"

#define PIX(x) (x < 0 ? 0 : (x > 255 ? 255 : x))

int main(int argc, char **argv)
{
    if(argc != 4){
        std::cout << "usage: " << argv[0] << " <quantizer (1-64; lower is better)> <input.raw> <output_dir>\n";
        return 0;
    }

    const int quantizer = std::stoi(argv[1]);
    const std::string input_filename = argv[2];
    const std::string output_dirname = argv[3];

    std::cout << "input: " << input_filename << "\n";
    std::cout << "ouput_dir: " << output_dirname << "\n";    

    const size_t width = 1280;
    const size_t height = 720;
    const size_t frame_size = width*height + width*height / 2;

    std::ifstream infile(input_filename, std::ios::binary);
    if(!infile.is_open()){
        std::cout << "Could not open file: " << input_filename << "\n";
        return 0;
    }
    
    std::shared_ptr<uint8_t[]> input_buffer(new uint8_t[frame_size]);
    std::shared_ptr<uint8_t[]> output_buffer(new uint8_t[frame_size]);

    // std::shared_ptr<uint8_t> y_buffer1(new uint8_t[frame_size/4]);
    // std::shared_ptr<uint8_t> u_buffer1(new uint8_t[frame_size/8]);
    // std::shared_ptr<uint8_t> v_buffer1(new uint8_t[frame_size/8]);
    // std::shared_ptr<uint8_t> y_buffer2(new uint8_t[frame_size/4]);
    // std::shared_ptr<uint8_t> u_buffer2(new uint8_t[frame_size/8]);
    // std::shared_ptr<uint8_t> v_buffer2(new uint8_t[frame_size/8]);
    
    // uint8_t *yuv_input[] = {y_buffer1.get(), u_buffer1.get(), v_buffer1.get()};
    // uint8_t *yuv_output[] = {y_buffer2.get(), u_buffer2.get(), v_buffer2.get()};

    H264_encoder encoder(width, height, quantizer);
    H264_encoder decoder(width, height);

    size_t frame_count = 0;
    while(!infile.eof()){
        infile.read((char*)input_buffer.get(), frame_size);

        // TODO: add code to encode the frame
        size_t compressed_frame_size = encoder.encode(input_buffer, output_buffer);
        
        { // scope the file object
            std::stringstream ss;
            ss << std::setw(5) << std::setfill('0') << frame_count;
            std::string output_filename = output_dirname + "/" + ss.str() + ".raw";
            
            std::ofstream outfile(output_filename, std::ios::binary);
            if(!outfile.is_open()){
                std::cout << "Could not open file: " << input_filename << "\n";
                return 0;
            }
            outfile.write((char*)output_buffer.get(), compressed_frame_size);
            frame_count++;
        }
    }

    return 0;
}
