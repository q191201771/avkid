#include <stdio.h>
#include <thread>
#include "avkid.hpp"

using namespace avkid;

int main(int argc, char **argv) {
  if (argc < 4) {
    AVKID_LOG_ERROR << "Usage: " << argv[0] << " <left> <right> <duraton>\n";
    return -1;
  }
  std::string left = argv[1];
  std::string right = argv[2];
  int duration = atoi(argv[3]);

  int iret = -1;

  global_init_ffmpeg();

  DecodePtr left_decode = std::make_shared<Decode>();
  InputPtr left_input = std::make_shared<Input>();
  AVKID_BIND_INPUT_TO_DECODE(left_input, left_decode);
  if (!left_input->open(left)) {
    AVKID_LOG_ERROR << "Open " << left << " failed.\n";
    return -1;
  }
  left_decode->open(left_input->in_fmt_ctx());

  left_input->read(duration * 1000);

  return 0;
}
