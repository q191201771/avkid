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
    AVKID_LOG_ERROR << "Usage: " << argv[0] << " <in> <out> <duraton>\n";
    return -1;
  }
  std::string in = argv[1];
  std::string out = argv[2];
  int duration = atoi(argv[3]);

  int iret = -1;

  global_init_ffmpeg();

  Output *output = new Output(false);
  Encode *encode = new Encode(std::bind(&Output::do_packet, output, std::placeholders::_1, std::placeholders::_2), false);
  Filter *filter = new Filter(std::bind(&Encode::do_frame, encode, std::placeholders::_1, std::placeholders::_2));
  Decode *decode = new Decode(std::bind(&Filter::do_frame, filter, std::placeholders::_1, std::placeholders::_2), false);
  Input *input = new Input(std::bind(&Decode::do_packet, decode, std::placeholders::_1, std::placeholders::_2));
  if (!input->open(in)) {
    AVKID_LOG_ERROR << "Open " << in << " failed.\n";
    return -1;
  }
  filter->open(input->in_fmt_ctx());
  decode->open(input->in_fmt_ctx());
  encode->open(input->in_fmt_ctx());
  output->open(out, input->in_fmt_ctx());

  input->read(duration * 1000);

  delete input;
  delete decode;
  delete filter;
  delete encode;
  delete output;

  return 0;
}
