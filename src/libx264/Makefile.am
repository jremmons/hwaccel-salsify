AM_CPPFLAGS = -I$(srcdir)/../../third_party/ffmpeg -I$(srcdir)/../util $(CXX14_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = test_coders sender receiver

test_coders_SOURCES = test_coders.cc h264_encoder.cc h264_decoder.cc
test_coders_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
test_coders_LDFLAGS = -pthread -ldl -lm

sender_SOURCES = sender.cc h264_encoder.cc
sender_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
sender_LDFLAGS = -pthread -ldl -lm

receiver_SOURCES = receiver.cc h264_decoder.cc
receiver_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
receiver_LDFLAGS = -pthread -ldl -lm

