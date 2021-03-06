#include <stdio.h>
#include "avkid.hpp"

using namespace avkid;

std::string g_in_url;
int g_jpeg_count = 0;
int g_jpeg_total;
int g_width;
int g_height;
InputPtr g_input;

std::string filename_flv = "output.flv";
std::string filename_mp4 = "output.mp4";
std::string filename_m3u8 = "output.m3u8";

std::string jpeg_filename() {
  char buf[128] = {0};
  snprintf(buf, 127, "./out_rtmp2jpeg_%llu_%d.jpeg", chef::stuff_op::unix_timestamp_msec(), g_jpeg_count);
  return std::string(buf);
}

class DumpJpeg : public FrameConsumerInterface {
  public:
    virtual void do_data(AVFrame *frame, bool is_audio) {
      if (is_audio || frame->key_frame != 1) { return; }

      g_jpeg_count++;
      if (g_jpeg_count == g_jpeg_total) { g_input->stop_read(); }
      else if (g_jpeg_count > g_jpeg_total) { return; }

      if (g_width == 0 || g_height == 0) {
        HelpOP::dump_mjpeg(frame, jpeg_filename());
      } else {
        AVFrame *dst_frame = HelpOP::scale_video_frame(frame, g_width, g_height);
        //AVFrame *dst_frame = HelpOP::cut_video_frame(frame, 100, 100, g_width, g_height);
        HelpOP::dump_mjpeg(dst_frame, jpeg_filename());
        HelpOP::frame_free_prop_unref_buf(&dst_frame);
      }
    }
};

int main(int argc, char **argv) {
  uint64_t bt = chef::stuff_op::tick_msec();
  {
    if (argc < 5) {
      std::cerr << "Usage: " << argv[0] << " <rtmp url> <num of jpeg> <width:set 0 to copy> <height:set 0 to copy>\n";
      return -1;
    }

    g_in_url = argv[1];
    g_jpeg_total = atoi(argv[2]);
    g_width = atoi(argv[3]);
    g_height = atoi(argv[4]);

    HelpOP::global_init_ffmpeg();

    g_input = Input::create();

    auto g_decode = Decode::create();
    //auto g_filter = Filter::create();
    auto dump_jpeg = std::make_shared<DumpJpeg>();

    auto output_flv = Output::create();
    auto output_mp4 = Output::create();
    auto output_m3u8 = Output::create();

    auto bc = std::make_shared<PacketBroadcast>();

    combine(g_input, bc);

    // 用于解码后生成jpeg图片
    bc->add_data_handler(g_decode);
    //combine(g_decode, g_filter, dump_jpeg);
    combine(g_decode, dump_jpeg);

    // 录制flv mp4 m3u8三份文件
    bc->add_data_handler(output_flv);
    bc->add_data_handler(output_mp4);
    bc->add_data_handler(output_m3u8);

    if (!g_input->open(g_in_url)) {
      AVKID_LOG_ERROR << "open " << g_in_url << " failed.\n";
      return -1;
    }

    g_decode->open(g_input->in_fmt_ctx());
    //g_filter->open(g_input->in_fmt_ctx());

    output_flv->open(filename_flv, g_input->in_fmt_ctx());
    output_mp4->open(filename_mp4, g_input->in_fmt_ctx());
    output_m3u8->open(filename_m3u8, g_input->in_fmt_ctx());

    g_input->read();
  }
  std::cerr << "Cost:" << chef::stuff_op::tick_msec() - bt << "\n";
  return 0;
}
