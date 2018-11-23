#include <stdio.h>
#include "avkid_input.h"
#include "avkid_decode.h"
#include "avkid_log_adapter.hpp"

using namespace avkid;

Input *g_input = nullptr;
Decode *g_decode = nullptr;

std::string g_in_url;
int g_jpeg_total;
bool g_decode_async_mode;

int g_jpeg_count = 0;

std::string jpeg_filename() {
  if (g_jpeg_count++ == g_jpeg_total) { exit(0); }

  char buf[128] = {0};
  snprintf(buf, 127, "./resource/out_rtmp2jpeg_%llu_%d.jpeg", chef::stuff_op::unix_timestamp_msec(), g_jpeg_count);
  return std::string(buf);
}

class DecodeObServerImpl : public DecodeObserver {
  public:
    virtual void frame_cb(AVFrame *av_frame, bool is_audio) {
      if (is_audio || av_frame->key_frame != 1) { return; }

      AVKID_LOG_DEBUG << "> dump mjpeg.\n";
      dump_mjpeg(av_frame, jpeg_filename());
    }

};

int main(int argc, char **argv) {
  if (argc < 4) {
    AVKID_LOG_ERROR << "Usage: " << argv[0] << " <rtmp url> <num of jpeg> <decode async mode>\n";
    return -1;
  }

  global_init_ffmpeg();

  g_in_url = argv[1];
  g_jpeg_total = atoi(argv[2]);
  g_decode_async_mode = atoi(argv[3]);

  DecodeObServerImpl *doi = new DecodeObServerImpl();
  g_decode = new Decode(doi, g_decode_async_mode);
  g_input = new Input(g_decode);
  if (!g_input->open(g_in_url)) {
    AVKID_LOG_ERROR << "open " << g_in_url << " failed.\n";
    return -1;
  }
  g_decode->open(g_input->in_fmt_ctx());
  g_input->read();

  delete g_input;
  delete g_decode;
  delete doi;
  return 0;
}
