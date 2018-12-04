#include <stdio.h>
#include <thread>
#include "avkid.hpp"

using namespace avkid;

int main(int argc, char **argv) {
  uint64_t bt = chef::stuff_op::tick_msec();
  {
    if (argc < 8) {
      AVKID_LOG_ERROR << "Usage: " << argv[0] << " <in> <out> <duraton> <decode async> <filter async> <encode async> <output async>\n";
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

    global_init_ffmpeg();

    OutputPtr output = std::make_shared<Output>(decode_async);
    EncodePtr encode = std::make_shared<Encode>(filter_async);
    FilterPtr filter = std::make_shared<Filter>(encode_async);
    DecodePtr decode = std::make_shared<Decode>(output_async);
    InputPtr input = std::make_shared<Input>();
    AVKID_BIND_INPUT_TO_DECODE(input, decode);
    AVKID_BIND_DECODE_TO_FILTER(decode, filter);
    AVKID_BIND_FILTER_TO_ENCODE(filter, encode);
    AVKID_BIND_ENCODE_TO_OUTPUT(encode, output);
    if (!input->open(in)) {
      AVKID_LOG_ERROR << "Open " << in << " failed.\n";
      return -1;
    }
    filter->open(input->in_fmt_ctx());
    decode->open(input->in_fmt_ctx());
    encode->open(input->in_fmt_ctx());
    output->open(out, input->in_fmt_ctx());

    input->read(duration * 1000);
  }

  std::cerr << "Cost:" << chef::stuff_op::tick_msec() - bt << "\n";
  return 0;
}
