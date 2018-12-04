#include <stdio.h>
#include "avkid.hpp"

using namespace avkid;

std::string g_in_url;
int g_jpeg_count = 0;
int g_jpeg_total;
int g_width;
int g_height;
bool g_decode_async_mode;
bool g_filter_async_mode;
InputPtr g_input;

std::string jpeg_filename() {
  char buf[128] = {0};
  snprintf(buf, 127, "./out_rtmp2jpeg_%llu_%d.jpeg", chef::stuff_op::unix_timestamp_msec(), g_jpeg_count);
  return std::string(buf);
}

void frame_cb(AVFrame *av_frame, bool is_audio) {
  if (is_audio || av_frame->key_frame != 1) { return; }

  g_jpeg_count++;
  if (g_jpeg_count == g_jpeg_total) { g_input->stop_read(); }
  else if (g_jpeg_count > g_jpeg_total) { return; }

  if (g_width == 0 || g_height == 0) {
    dump_mjpeg(av_frame, jpeg_filename());
  } else {
    AVFrame *dst_frame = scale_video_frame(av_frame, g_width, g_height);
    dump_mjpeg(dst_frame, jpeg_filename());
  }
}

int main(int argc, char **argv) {
  uint64_t bt = chef::stuff_op::tick_msec();
  {
    if (argc < 7) {
      std::cerr << "Usage: " << argv[0] << " <rtmp url> <width:set 0 to copy> <height:set 0 to copy> <num of jpeg> <decode async mode> <filter async mode>\n";
      return -1;
    }

    g_in_url = argv[1];
    g_jpeg_total = atoi(argv[2]);
    g_width = atoi(argv[3]);
    g_height = atoi(argv[4]);
    g_decode_async_mode = atoi(argv[5]);
    g_filter_async_mode = atoi(argv[6]);

    global_init_ffmpeg();

    g_input = std::make_shared<Input>();
    DecodePtr g_decode = std::make_shared<Decode>(g_decode_async_mode);
    FilterPtr g_filter = std::make_shared<Filter>(g_filter_async_mode);
    g_filter->set_frame_handler(std::bind(&frame_cb, std::placeholders::_1, std::placeholders::_2));

    AVKID_BIND_DECODE_TO_FILTER(g_decode, g_filter);
    AVKID_BIND_INPUT_TO_DECODE(g_input, g_decode);

    if (!g_input->open(g_in_url)) {
      AVKID_LOG_ERROR << "open " << g_in_url << " failed.\n";
      return -1;
    }
    g_decode->open(g_input->in_fmt_ctx());
    g_filter->open(g_input->in_fmt_ctx());
    g_input->read();
  }
  std::cerr << "Cost:" << chef::stuff_op::tick_msec() - bt << "\n";
  return 0;
}
