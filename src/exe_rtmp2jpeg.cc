#include <stdio.h>
#include "avkid_input.h"
#include "avkid_decode.h"
#include "avkid_filter.h"
#include "avkid_log_adapter.hpp"

using namespace avkid;

std::string g_in_url;
int g_jpeg_total;
int g_width;
int g_height;
bool g_decode_async_mode;

Filter *g_filter = nullptr;

std::string jpeg_filename() {
  static int jpeg_count = 0;
  if (jpeg_count++ == g_jpeg_total) { exit(0); }

  char buf[128] = {0};
  snprintf(buf, 127, "./resource/out_rtmp2jpeg_%llu_%d.jpeg", chef::stuff_op::unix_timestamp_msec(), jpeg_count);
  return std::string(buf);
}

void frame_cb(AVFrame *av_frame, bool is_audio) {
  if (is_audio || av_frame->key_frame != 1) { return; }

  if (g_width == 0 || g_height == 0) {
    dump_mjpeg(av_frame, jpeg_filename());
  } else {
    AVFrame *dst_frame = scale_video_frame(av_frame, g_width, g_height);
    dump_mjpeg(dst_frame, jpeg_filename());
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <rtmp url> <width:set 0 to copy> <height:set 0 to copy> <num of jpeg> <decode async mode>\n";
    return -1;
  }

  g_in_url = argv[1];
  g_jpeg_total = atoi(argv[2]);
  g_width = atoi(argv[3]);
  g_height = atoi(argv[4]);
  g_decode_async_mode = atoi(argv[5]);

  global_init_ffmpeg();

  // TODO erase g_ prefix.
  Input *g_input = nullptr;
  Decode *g_decode = nullptr;

  g_filter = new Filter(std::bind(&frame_cb, std::placeholders::_1, std::placeholders::_2));
  g_decode = new Decode(std::bind(&Filter::do_frame, g_filter, std::placeholders::_1, std::placeholders::_2), g_decode_async_mode);
  g_input = new Input(std::bind(&Decode::do_packet, g_decode, std::placeholders::_1, std::placeholders::_2));
  if (!g_input->open(g_in_url)) {
    AVKID_LOG_ERROR << "open " << g_in_url << " failed.\n";
    return -1;
  }
  g_decode->open(g_input->in_fmt_ctx());
  g_filter->open(g_input->in_fmt_ctx());
  g_input->read();

  delete g_input;
  delete g_decode;
  delete g_filter;
  return 0;
}
