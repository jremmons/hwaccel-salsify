AM_CPPFLAGS = -I$(srcdir)/../../third_party/ffmpeg -I$(srcdir)/../util $(CXX14_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = test_coders ssender sreceiver

test_coders_SOURCES = test_coders.cc h264_encoder.cc h264_decoder.cc
test_coders_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
test_coders_LDFLAGS = -pthread -ldl -lm

ssender_SOURCES = ssender.cc h264_encoder.cc h264_decoder.cc
ssender_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
ssender_LDFLAGS = -pthread -ldl -lm

sreceiver_SOURCES = sreceiver.cc h264_encoder.cc h264_decoder.cc
sreceiver_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
sreceiver_LDFLAGS = -pthread -ldl -lm
