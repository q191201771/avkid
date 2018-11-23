CXX = g++
CXXFLAGS = -g -O0 -std=c++11 -pipe -fPIC # -Wall

LINKFLAGS = -lavformat -lavcodec -lavutil -lavfilter -lswscale -lfdk-aac

all: rtmp2jpeg rtmpdump tag_nalus tail #  rtmp2flv

COMMON_HEADER = $(wildcard src/*.h)
COMMON_HEADER += $(wildcard src/*.hpp)

COMMON_SRC = src/avkid_input.cc src/avkid_decode.cc src/avkid_output.cc #src/avkid_in.cc src/avkid_out.cc

RTMP_2_JPEG_SRC = src/exe_rtmp2jpeg.cc $(COMMON_SRC)
RTMP_2_FLV_SRC = src/exe_rtmp2flv.cc $(COMMON_SRC)
RTMPDUMP_SRC = src/exe_rtmpdump.cc $(COMMON_SRC)
TAG_NALUS_SRC = src/exe_tag_nalus.cc $(COMMON_SRC)

rtmp2jpeg: $(RTMP_2_JPEG_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp2jpeg $(RTMP_2_JPEG_SRC) $(LINKFLAGS)

rtmp2flv: $(RTMP_2_FLV_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp2flv $(RTMP_2_FLV_SRC) $(LINKFLAGS)

rtmpdump: $(RTMPDUMP_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmpdump $(RTMPDUMP_SRC) $(LINKFLAGS)

tag_nalus: $(TAG_NALUS_SRC) $(COMMON_HEADER)
	$(CXX) $(CXXFLAGS) -o tag_nalus $(TAG_NALUS_SRC) $(LINKFLAGS)

tail:
	rm -rf rtmp2jpeg.dSYM | date | ls -l rtmp2jpeg
	rm -rf rtmp2flv.dSYM | date | ls -l rtmp2flv
	rm -rf rtmpdump.dSYM/ | date | ls -l rtmpdump
	rm -rf tag_nalus.dSYM | date | ls -l tag_nalus

clean:
	rm -rf rtmp2jpeg rtmp2flv rtmpdump tag_nalus

