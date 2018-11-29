#include <stdio.h>
#include <thread>
#include "avkid_common.hpp"
#include "avkid_input.h"
#include "avkid_output.h"

using namespace avkid;

int main(int argc, char **argv) {
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

  Output *output = new Output(output_async_mode);
  Input *input = new Input(output);
  if (!input->open(in)) {
    AVKID_LOG_ERROR << "Open " << in << " failed.\n";
    return -1;
  }

  output->open(out, input->in_fmt_ctx());

  input->read(duration_sec * 1000);

  delete output;
  delete input;

  return 0;
}
