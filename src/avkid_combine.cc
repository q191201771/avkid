#include "avkid_combine.h"
#include "avkid.hpp"

namespace avkid {

DecodePtr combine(InputPtr in, DecodePtr out) {
  AVKID_COMBINE_MODULE_CC(in, out, &Decode::do_data);
  return out;
}

OutputPtr combine(InputPtr in, OutputPtr out) {
  AVKID_COMBINE_MODULE_CC(in, out, &Output::do_data);
  return out;
}

FilterPtr combine(DecodePtr in, FilterPtr out) {
  AVKID_COMBINE_MODULE_CC(in, out, &Filter::do_data);
  return out;
}

EncodePtr combine(DecodePtr in, EncodePtr out) {
  AVKID_COMBINE_MODULE_CC(in, out, &Encode::do_data);
  return out;
}

OutputPtr combine(EncodePtr in, OutputPtr out) {
  AVKID_COMBINE_MODULE_CC(in, out, &Output::do_data);
  return out;
}

EncodePtr combine(FilterPtr in, EncodePtr out) {
  AVKID_COMBINE_MODULE_CC(in, out, &Encode::do_data);
  return out;
}

void PacketProducer::set_data_handler(PacketHandlerT ph) {
  ph_ = ph;
}

void FrameProducer::set_data_handler(FrameHandlerT fh) {
  fh_ = fh;
}

}

