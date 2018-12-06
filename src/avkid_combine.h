/**
 * @file   avkid_combine.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

#define AVKID_COMBINE_MODULE_CC(in, out, func) (in)->set_data_handler(std::bind(func, out, std::placeholders::_1, std::placeholders::_2));
#define AVKID_COMBINE_MODULE_C(in, func)       (in)->set_data_handler(std::bind(func, std::placeholders::_1, std::placeholders::_2));

DecodePtr combine(InputPtr in, DecodePtr out);
OutputPtr combine(InputPtr in, OutputPtr out);
FilterPtr combine(DecodePtr in, FilterPtr out);
EncodePtr combine(DecodePtr in, EncodePtr out);
OutputPtr combine(EncodePtr in, OutputPtr out);
EncodePtr combine(FilterPtr in, EncodePtr out);

class PacketProducer {
  public:
    void set_data_handler(PacketHandlerT ph);

  protected:
    PacketHandlerT ph_;
};

class FrameProducer {
  public:
    void set_data_handler(FrameHandlerT fh);

  protected:
    FrameHandlerT fh_;
};

}
