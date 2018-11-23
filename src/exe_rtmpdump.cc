#include <stdio.h>
#include <thread>
#include "avkid_common.hpp"
#include "avkid_input.h"
#include "avkid_output.h"

using namespace avkid;

int main(int argc, char **argv) {
  if (argc < 4) {
    AVKID_LOG_ERROR << "Usage: " << argv[0] << " <in> <out> <output_async_mode>\n";
    return -1;
  }
  std::string in = argv[1];
  std::string out = argv[2];
  bool output_async_mode = atoi(argv[3]);

  int iret = -1;

  global_init_ffmpeg();

  Output *output = new Output(output_async_mode);
  Input *input = new Input(output);
  if (!input->open(in)) {
    AVKID_LOG_ERROR << "Open " << in << " failed.\n";
    return -1;
  }

  output->open(out, input->in_fmt_ctx());

  std::thread thd([input] {
      sleep(10);
      input->stop_read();
  });
  thd.detach();

  input->read();

  delete output;
  delete input;

  return 0;
}
