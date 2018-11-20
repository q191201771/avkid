CXX = g++
CXXFLAGS = -g -O0 -std=c++11 -pipe -fPIC # -Wall

LINKFLAGS = -lavformat -lavcodec -lavutil -lavfilter -lswscale -lfdk-aac

all: rtmp2jpeg rtmp2flv

COMMON_HEADER = $(wildcard src/*.h)
COMMON_HEADER += $(wildcard src/*.hpp)

COMMON_SRC = src/avkid_in.cc src/avkid_out.cc

RTMP_2_JPEG_SRC = src/exe_rtmp2jpeg.cc $(COMMON_SRC)
RTMP_2_FLV_SRC = src/exe_rtmp2flv.cc $(COMMON_SRC)

rtmp2jpeg: $(RTMP_2_JPEG_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp2jpeg $(RTMP_2_JPEG_SRC) $(LINKFLAGS)
	rm -rf rtmp2jpeg.dSYM | date | ls -l rtmp2jpeg

rtmp2flv: $(RTMP_2_FLV_SRC) $(COMMON_HEADER) $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o rtmp2flv $(RTMP_2_FLV_SRC) $(LINKFLAGS)
	rm -rf rtmp2flv.dSYM | date | ls -l rtmp2flv

clean:
	echo $(COMMON_HEADER)
	rm -rf rtmp2jpeg rtmp2flv

