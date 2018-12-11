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

    auto output = Output::create(decode_async);
    auto encode = Encode::create(filter_async);
    auto filter = Filter::create(encode_async);
    auto decode = Decode::create(output_async);
    auto input = Input::create();

    combine(combine(combine(combine(input, decode), filter), encode), output);
    //combine(combine(combine(input, decode), encode), output);

    if (!input->open(in)) {
      AVKID_LOG_ERROR << "Open " << in << " failed.\n";
      return -1;
    }
    decode->open(input->in_fmt_ctx());
    filter->open(input->in_fmt_ctx());
    encode->open(input->in_fmt_ctx());
    output->open(out, input->in_fmt_ctx());

    input->read(duration * 1000);
  }

  std::cerr << "Cost:" << chef::stuff_op::tick_msec() - bt << "\n";
  return 0;
}
