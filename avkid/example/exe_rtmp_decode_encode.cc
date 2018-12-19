#include <stdio.h>
#include <thread>
#include "avkid.hpp"

using namespace avkid;

int main(int argc, char **argv) {
  uint64_t bt = chef::stuff_op::tick_msec();
  {
    if (argc < 8) {
      std::cerr << "Usage: " << argv[0] << " <in> <out> <duraton> <decode async> <filter async> <encode async> <output async>\n";
      return -1;
    }
    std::string in = argv[1];
    std::string out = argv[2];
    int duration = atoi(argv[3]);
    bool decode_async = atoi(argv[4]);
    bool filter_async = atoi(argv[5]);
    bool encode_async = atoi(argv[6]);
    bool output_async = atoi(argv[7]);

    int iret = -1;

    HelpOP::global_init_ffmpeg();

    auto output = Output::create(output_async);
    auto encode = Encode::create(encode_async);
    auto filter = Filter::create(decode_async);
    auto decode = Decode::create(filter_async);
    auto input = Input::create();

    //const char *filter_descr = "drawtext=\"text='Test Text'\"";
    //const char *filter_descr = "boxblur=20:1:cr=0:ar=0";
    // 画一个框
    //const char *filter_descr = "drawbox=x=10:y=20:w=200:h=60:color=red@0.5";
    // 控制yuv值，类似黑白效果
    const char *filter_descr = "lutyuv='u=128:v=128'";
    // 左右翻转
    //const char *filter_descr = "hflip";
    // 上下翻转
    //const char *filter_descr = "vflip";


    combine(input, decode, encode, output);
    //combine(input, decode, filter, encode, output);

    if (!input->open(in)) {
      AVKID_LOG_ERROR << "Open " << in << " failed.\n";
      return -1;
    }
    decode->open(input->in_fmt_ctx());
    filter->open(input->in_fmt_ctx(), filter_descr);
    encode->open(input->in_fmt_ctx());
    output->open(out, input->in_fmt_ctx());

    input->read(duration * 1000);
  }

  std::cerr << "Cost:" << chef::stuff_op::tick_msec() - bt << "\n";
  return 0;
}
