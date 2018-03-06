#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "h264_encoder.hh"
#include "h264_decoder.hh"

#define PIX(x) (x < 0 ? 0 : (x > 255 ? 255 : x))

int main(int argc, char **argv)
{
    if(argc != 4){
        std::cout << "usage: " << argv[0] << " <quantizer (1-64; lower is better)> <input.raw> <output.raw>\n";
        return 0;
    }

    const int quantizer = std::stoi(argv[1]);
    const std::string input_filename = argv[2];
    const std::string output_filename = argv[3];

    std::cout << "input: " << input_filename << "\n";
    std::cout << "ouput: " << output_filename << "\n";    

    const size_t width = 1280;
    const size_t height = 720;
    const size_t frame_size = width*height + width*height / 2;

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

    auto buffer1 = std::move(std::shared_ptr<uint8_t[]>((uint8_t*)aligned_alloc(32,frame_size)));;
    auto buffer2 = std::move(std::shared_ptr<uint8_t[]>((uint8_t*)aligned_alloc(32,frame_size)));;
    auto buffer3 = std::move(std::shared_ptr<uint8_t[]>((uint8_t*)aligned_alloc(32,frame_size)));;

    H264_encoder encoder(width, height, quantizer);
    H264_decoder decoder(width, height);

    size_t frame_count = 0;
    while(!infile.eof()){

        // read in raw video
        infile.read((char*)buffer1.get(), frame_size);

        // encode
        size_t compressed_frame_size = encoder.encode(buffer1, buffer2);

        // decode
        decoder.decode(buffer2, compressed_frame_size, buffer3);

        // write out raw video
        outfile.write((char*)buffer3.get(), frame_size);
        frame_count++;
    }

    return 0;
}
