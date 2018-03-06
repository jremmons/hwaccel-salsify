#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "h264_decoder.hh"

#define PIX(x) (x < 0 ? 0 : (x > 255 ? 255 : x))

int main(int argc, char **argv)
{
    if(argc <= 2){
        std::cout << "usage: " << argv[0] << "<output.raw> <input.compressed> [... input.compressed]\n";
        return 0;
    }

    const std::string output_filename = argv[1];
    const size_t inputs_arg_start_idx = 2;
    const size_t inputs_nframes = argc - 1;

    std::cout << "ouput: " << output_filename << "\n";    
    for(int i = inputs_arg_start_idx; i < inputs_nframes+inputs_arg_start_idx; i++){
        std::cout << "input: " << argv[i] << "\n";
        break;
    }

    const size_t width = 1280;
    const size_t height = 720;
    const size_t frame_size = width*height + width*height / 2;
    
    std::ofstream outfile(output_filename, std::ios::binary);
    if(!outfile.is_open()){
        std::cout << "Could not open file: " << output_filename << "\n";
        return 0;
    }
    
    std::shared_ptr<uint8_t[]> buffer1(new uint8_t[frame_size]);
    std::shared_ptr<uint8_t[]> buffer2(new uint8_t[frame_size]);

    H264_decoder decoder(width, height);

    for(int i = inputs_arg_start_idx; i < inputs_nframes+inputs_arg_start_idx; i++){    

        size_t compressed_size = 0;
        { // scope the file object            
            std::ifstream infile(argv[i], std::ios::binary);
            if(!infile.is_open()){
                std::cout << "Could not open file: " << argv[i] << "\n";
                return 0;
            }

            infile.seekg(0, std::ios::end);
            compressed_size = infile.tellg();
            infile.seekg(0, std::ios::beg);
            
            infile.read((char*)buffer1.get(), compressed_size);
        }

        decoder.decode(buffer1.get(), compressed_size, buffer2.get());
        
        outfile.write((char*)buffer2.get(), frame_size);
    }

    return 0;
}
