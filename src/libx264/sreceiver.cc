/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <memory>

#include "optional.hh"
#include "h264_encoder.hh"
#include "h264_decoder.hh"

using namespace std;

constexpr uint16_t width = 1280;
constexpr uint16_t height = 720;
constexpr uint32_t raster_size = ( width * height * 3 ) / 2;

typedef vector<uint8_t> Raster;
typedef vector<uint8_t> Frame;

void usage()
{
  cerr << "sender <input.compressed> <output.raw> <trace>" << endl;
}

int main( int argc, char const * argv[] )
{
  if ( argc != 4 ) {
    usage();
    return EXIT_FAILURE;
  }

  Frame frame_buffer;
  frame_buffer.resize( raster_size );

  /* open the i/o streams */
  ifstream fin { argv[ 1 ] };
  ofstream fout { argv[ 2 ] };
  ifstream trace_fin { argv[ 3 ] };

  /* the decoder objects */
  unique_ptr<H264_decoder> decoder;

  Optional<Raster> prev_raster;

  Frame temp_frame;
  temp_frame.resize( raster_size );
  size_t temp_frame_size = 0;

  size_t winner = 0;
  size_t prev_winner = 0;

  while ( not fin.eof() ) {
    trace_fin >> winner;

    /* read the compressed frame */
    size_t frame_size;
    size_t frame_q;
    fin.read( reinterpret_cast<char *>( &frame_size ), sizeof( frame_size ) );
    fin.read( reinterpret_cast<char *>( &frame_q ), sizeof( frame_q ) );
    fin.read( reinterpret_cast<char *>( frame_buffer.data() ), frame_size );

    if ( not prev_raster.initialized() ) {
      prev_raster.reset();
      prev_raster->resize( raster_size );
      decoder.reset( new H264_decoder( width, height ) );
    }
    else {
      if ( prev_winner != winner ) {
        H264_encoder encoder { width, height, frame_q };
        temp_frame_size = encoder.encode( prev_raster->data(), temp_frame.data() );
        decoder.reset( new H264_decoder( width, height ) );
        decoder->decode( temp_frame.data(), temp_frame_size, prev_raster->data() );
      }
    }

    decoder->decode( frame_buffer.data(), frame_size, prev_raster->data() );
    fout.write( reinterpret_cast<char *>( prev_raster->data() ), raster_size );

    prev_winner = winner;
  }

  return 0;
}
