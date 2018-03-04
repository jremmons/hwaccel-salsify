AM_CPPFLAGS = -I$(srcdir)/../../third_party/ffmpeg -I$(srcdir)/../util $(CXX14_FLAGS)
AM_CXXFLAGS = $(PICKY_CXXFLAGS)

bin_PROGRAMS = degrade_raw_video

degrade_raw_video_SOURCES = degrade_raw_video.cc h264_degrader.cc
degrade_raw_video_LDADD = $(AVFORMAT_LIBS) $(AVCODEC_LIBS) $(AVUTIL_LIBS) $(AVFILTER_LIBS) $(AVDEVICE_LIBS)
degrade_raw_video_LDFLAGS = -pthread -ldl -lm
