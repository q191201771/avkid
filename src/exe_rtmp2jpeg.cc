#include <stdio.h>
#include "avkid_in.h"
#include "avkid_log_adapter.hpp"

using namespace avkid;

In *g_in;

std::string g_in_url;
int g_jpeg_total;

int g_jpeg_count = 0;

std::string jpeg_filename() {
  if (g_jpeg_count++ == g_jpeg_total) { exit(0); }

  char buf[128] = {0};
  snprintf(buf, 127, "./resource/out_rtmp2jpeg_%llu_%d.jpeg", chef::stuff_op::unix_timestamp_msec(), g_jpeg_count);
  return std::string(buf);
}

class InObserverImpl : public InObserver {
  public:
    virtual void packet_cb(AVPacket *av_packet, bool is_audio) {  }

    virtual void frame_cb(AVFrame *av_frame, bool is_audio) {
      if (is_audio || av_frame->key_frame != 1) { return; }

      AVKID_LOG_DEBUG << "> dump mjpeg.\n";
      dump_mjpeg(av_frame, jpeg_filename());
    }

};

int main(int argc, char **argv) {
  if (argc != 3) {
    AVKID_LOG_ERROR << "Usage: " << argv[0] << " <rtmp url> <num of jpeg>\n";
    return -1;
  }

  global_init_ffmpeg();

  g_in_url = argv[1];
  g_jpeg_total = atoi(argv[2]);

  InObserverImpl ioi;
  g_in = new In(&ioi);
  if (!g_in->open(g_in_url)) {
    AVKID_LOG_ERROR << "open " << g_in_url << " failed.\n";
    return -1;
  }
  g_in->read();
  delete g_in;

  return 0;
}
