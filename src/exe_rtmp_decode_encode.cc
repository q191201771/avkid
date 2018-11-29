#include <stdio.h>
#include <thread>
#include "avkid_common.hpp"
#include "avkid_input.h"
#include "avkid_output.h"
#include "avkid_decode.h"
#include "avkid_encode.h"


using namespace avkid;

int main(int argc, char **argv) {
  if (argc < 4) {
    AVKID_LOG_ERROR << "Usage: " << argv[0] << " <in> <out> <duraton>\n";
    return -1;
  }
  std::string in = argv[1];
  std::string out = argv[2];
  int duration = atoi(argv[3]);

  int iret = -1;

  global_init_ffmpeg();

  Output *output = new Output(false);
  Encode *encode = new Encode(output, false);
  Decode *decode = new Decode(encode, false);
  Input *input = new Input(decode);
  if (!input->open(in)) {
    AVKID_LOG_ERROR << "Open " << in << " failed.\n";
    return -1;
  }
  decode->open(input->in_fmt_ctx());
  encode->open(input->in_fmt_ctx());
  output->open(out, input->in_fmt_ctx());

  //std::thread thd([input, duration] {
  //    sleep(duration);
  //    input->stop_read();
  //});
  //thd.detach();

  input->read(duration * 1000);

  delete input;
  delete decode;
  delete encode;
  delete output;

  return 0;
}
