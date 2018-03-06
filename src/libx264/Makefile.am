AM_CPPFLAGS = -I$(srcdir)/../../third_party/ffmpeg -I$(srcdir)/../util $(CXX14_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = test_coders test_degrader

test_coders_SOURCES = test_coders.cc h264_encoder.cc h264_decoder.cc
test_coders_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
test_coders_LDFLAGS = -pthread -ldl -lm

test_degrader_SOURCES = test_degrader.cc h264_degrader.cc 
test_degrader_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
test_degrader_LDFLAGS = -pthread -ldl -lm

