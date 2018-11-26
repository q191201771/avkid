CXX = g++
CXXFLAGS = -g -O0 -std=c++11 -pipe -fPIC # -Wall

LINKFLAGS = -lavformat -lavcodec -lavutil -lavfilter -lswscale -lfdk-aac

all: rtmp2jpeg rtmpdump rtmp_decode_encode tag_nalus tail

COMMON_HEADER = $(wildcard src/*.h)
COMMON_HEADER += $(wildcard src/*.hpp)

COMMON_SRC = src/avkid_input.cc src/avkid_decode.cc src/avkid_encode.cc src/avkid_output.cc

RTMP_2_JPEG_SRC = src/exe_rtmp2jpeg.cc $(COMMON_SRC)
RTMPDUMP_SRC = src/exe_rtmpdump.cc $(COMMON_SRC)
RTMP_DECODE_ENCODE_SRC = src/exe_rtmp_decode_encode.cc $(COMMON_SRC)
TAG_NALUS_SRC = src/exe_tag_nalus.cc $(COMMON_SRC)

rtmp2jpeg: $(RTMP_2_JPEG_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp2jpeg $(RTMP_2_JPEG_SRC) $(LINKFLAGS)

rtmpdump: $(RTMPDUMP_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmpdump $(RTMPDUMP_SRC) $(LINKFLAGS)

rtmp_decode_encode: $(RTMP_DECODE_ENCODE_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp_decode_encode $(RTMP_DECODE_ENCODE_SRC) $(LINKFLAGS)

tag_nalus: $(TAG_NALUS_SRC) $(COMMON_HEADER)
	$(CXX) $(CXXFLAGS) -o tag_nalus $(TAG_NALUS_SRC) $(LINKFLAGS)

tail:
	rm -rf rtmp2jpeg.dSYM | date | ls -l rtmp2jpeg
	rm -rf rtmpdump.dSYM/ | date | ls -l rtmpdump
	rm -rf rtmp_decode_encode.dSYM/ | date | ls -l rtmp_decode_encode
	rm -rf tag_nalus.dSYM | date | ls -l tag_nalus

clean:
	rm -rf rtmp2jpeg rtmpdump tag_nalus

