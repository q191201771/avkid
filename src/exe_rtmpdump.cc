#include <stdio.h>
#include <thread>
#include "avkid.hpp"

using namespace avkid;

int main(int argc, char **argv) {
  uint64_t bt = chef::stuff_op::tick_msec();
  {
    if (argc < 5) {
      AVKID_LOG_ERROR << "Usage: " << argv[0] << " <in> <out> <duration_sec> <output_async_mode>\n";
      return -1;
    }
    std::string in = argv[1];
    std::string out = argv[2];
    int duration_sec = atoi(argv[3]);
    bool output_async_mode = atoi(argv[4]);

    int iret = -1;

    global_init_ffmpeg();

    OutputPtr output = std::make_shared<Output>(output_async_mode);
    InputPtr input = std::make_shared<Input>();
    AVKID_BIND_INPUT_TO_OUTPUT(input, output);
    if (!input->open(in)) {
      AVKID_LOG_ERROR << "Open " << in << " failed.\n";
      return -1;
    }

    output->open(out, input->in_fmt_ctx());

    input->read(duration_sec * 1000);
  }

  std::cerr << "Cost:" << chef::stuff_op::tick_msec() - bt << "\n";
  return 0;
}
