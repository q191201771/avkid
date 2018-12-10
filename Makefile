CXX = g++
CXXFLAGS = -g -O2 -std=c++11 -pipe -fPIC -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS # -Wall

LINKFLAGS = -lavformat -lavcodec -lavutil -lavfilter -lswscale -lfdk-aac

all: rtmp2jpeg rtmpdump rtmp_decode_encode mix tag_nalus

COMMON_HEADER = $(wildcard src/*.h)
COMMON_HEADER += $(wildcard src/*.hpp)

COMMON_SRC = \
	src/avkid_module_base.cc \
	src/avkid_module_input.cc \
	src/avkid_module_decode.cc \
	src/avkid_module_filter.cc \
	src/avkid_module_encode.cc \
	src/avkid_module_output.cc \
	src/avkid_mix_op.cc \
	src/avkid_help_op.cc

RTMP_2_JPEG_SRC = src/exe_rtmp2jpeg.cc $(COMMON_SRC)
RTMPDUMP_SRC = src/exe_rtmpdump.cc $(COMMON_SRC)
RTMP_DECODE_ENCODE_SRC = src/exe_rtmp_decode_encode.cc $(COMMON_SRC)
MIX_SRC = src/exe_mix.cc $(COMMON_SRC)
TAG_NALUS_SRC = src/exe_tag_nalus.cc

rtmp2jpeg: $(RTMP_2_JPEG_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp2jpeg $(RTMP_2_JPEG_SRC) $(LINKFLAGS)

rtmpdump: $(RTMPDUMP_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmpdump $(RTMPDUMP_SRC) $(LINKFLAGS)

rtmp_decode_encode: $(RTMP_DECODE_ENCODE_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp_decode_encode $(RTMP_DECODE_ENCODE_SRC) $(LINKFLAGS)

mix: $(MIX_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o mix $(MIX_SRC) $(LINKFLAGS)

tag_nalus: $(TAG_NALUS_SRC) $(COMMO_HEADER)
	$(CXX) $(CXXFLAGS) -o tag_nalus $(TAG_NALUS_SRC) $(LINKFLAGS)

tail:
	rm -rf rtmp2jpeg.dSYM | date | ls -l rtmp2jpeg
	rm -rf rtmpdump.dSYM | date | ls -l rtmpdump
	rm -rf rtmp_decode_encode.dSYM | date | ls -l rtmp_decode_encode
	rm -rf mix.dSYM | date | ls -l mix
	rm -rf tag_nalus.dSYM | date | ls -l tag_nalus

clean:
	rm -rf rtmp2jpeg rtmpdump tag_nalus

