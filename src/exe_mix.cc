#include <stdio.h>
#include <thread>
#include "avkid_common.hpp"
#include "avkid_input.h"
#include "avkid_decode.h"
#include "avkid_filter.h"
#include "avkid_encode.h"
#include "avkid_output.h"


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

  Decode *left_decode = new Decode(nullptr, false);
  Input *left_input = new Input(std::bind(&Decode::do_packet, left_decode, std::placeholders::_1, std::placeholders::_2));
  if (!left_input->open(left)) {
    AVKID_LOG_ERROR << "Open " << left << " failed.\n";
    return -1;
  }
  left_decode->open(left_input->in_fmt_ctx());

  left_input->read(duration * 1000);

  delete left_input;
  delete left_decode;

  return 0;
}
