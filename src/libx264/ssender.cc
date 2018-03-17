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
constexpr uint32_t frame_size = ( width * height * 3 ) / 2;

constexpr uint16_t q_high = 16;
constexpr uint16_t q_low = 48;

typedef vector<uint8_t> Raster;
typedef vector<uint8_t> Frame;

void usage()
{
  cerr << "sender <input.raw> <output.compressed> <trace>" << endl;
}

int main( int argc, char const * argv[] )
{
  if ( argc != 4 ) {
    usage();
    return EXIT_FAILURE;
  }

  /* open the i/o streams */
  ifstream input_fin { argv[ 1 ] };
  ofstream fout { argv[ 2 ] };
  ifstream trace_fin { argv[ 3 ] };

  /* create the raster buffer */
  Raster raster_buffer;
  raster_buffer.resize( frame_size );

  /* create the frame buffers */
  Frame frame_buffer[ 2 ];
  frame_buffer[ 0 ].resize( frame_size );
  frame_buffer[ 1 ].resize( frame_size );

  /* the encoder objects */
  unique_ptr<H264_encoder> encoders [ 2 ];
  encoders[ 0 ].reset( new H264_encoder( width, height, q_high ) );
  encoders[ 1 ].reset( new H264_encoder( width, height, q_low ) );
  size_t out_frame_sizes[ 2 ] = { 0 };

  /* the decoder objects */
  unique_ptr<H264_decoder> decoder { new H264_decoder( width, height ) };

  Optional<Raster> winning_raster;

  Raster temp_raster;
  temp_raster.resize( frame_size );

  Frame temp_frame;
  temp_frame.resize( frame_size );
  size_t temp_frame_size = 0;

  uint16_t winner = 0;
  uint16_t prev_winner = 0;

  while ( not input_fin.eof() ) {
    trace_fin >> winner;

    /* read and encode the raster with two different qualities */
    input_fin.read( reinterpret_cast<char *>( raster_buffer.data() ), frame_size );
    out_frame_sizes[ 0 ] = encoders[ 0 ]->encode( raster_buffer.data(), frame_buffer[ 0 ].data() );
    out_frame_sizes[ 1 ] = encoders[ 1 ]->encode( raster_buffer.data(), frame_buffer[ 1 ].data() );
    fout.write( reinterpret_cast<char *>( &out_frame_sizes[ winner ] ), sizeof( out_frame_sizes[ winner ] ) );
    fout.write( reinterpret_cast<char *>( frame_buffer[ winner ].data() ), out_frame_sizes[ winner ] );

    if ( not winning_raster.initialized() ) {
      winning_raster.reset();
      winning_raster->resize( frame_size );
      decoder->decode( frame_buffer[ winner ].data(), out_frame_sizes[ winner ], winning_raster->data() );
    }
    else {
      if ( winner != prev_winner ) {
        decoder.reset( new H264_decoder( width, height ) );
        decoder->decode( temp_frame.data(), temp_frame_size, temp_raster.data() );
        decoder->decode( frame_buffer[ winner ].data(), out_frame_sizes[ winner ], winning_raster->data() );
      }
      else {
        decoder->decode( frame_buffer[ winner ].data(), out_frame_sizes[ winner ], winning_raster->data() );
      }
    }

    unique_ptr<H264_encoder> aux_encoder { new H264_encoder( width, height, encoders[ 1 - winner ]->q() ) };
    temp_frame_size = aux_encoder->encode( winning_raster->data(), temp_frame.data() );
    aux_encoder.swap( encoders[ 1 - winner ] );
    prev_winner = winner;
  }

  return 0;
}
