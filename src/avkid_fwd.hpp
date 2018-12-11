/**
 * @file   avkid_fwd.hpp
 * @author chef
 *
 */

#pragma once

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

extern "C" {
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/samplefmt.h>
  #include <libavutil/timestamp.h>
  #include <libavutil/imgutils.h>
  #include <libavutil/opt.h>
  #include <libavfilter/avfilter.h>
  #include <libavfilter/buffersink.h>
  #include <libavfilter/buffersrc.h>
  #include <libswscale/swscale.h>
}

#include <string>
#include <functional>

#include "chef_snippet.hpp"
#include "chef_task_thread.hpp"
#include "chef_strings_op.hpp"

namespace avkid {
  typedef std::function<void(AVPacket *, bool)> PacketHandlerT;
  typedef std::function<void(AVFrame *, bool)> FrameHandlerT;

  using std::placeholders::_1;
  using std::placeholders::_2;

  class PacketConsumerInterface;
  class FrameConsumerInterface;
  class HelpOP;
  class MixOP;
  class ModuleBase;
  class PacketProducer;
  class FrameProducer;
  class Input;
  class Decode;
  class Filter;
  class Encode;
  class Output;

  typedef std::shared_ptr<Input> InputPtr;
  typedef std::shared_ptr<Decode> DecodePtr;
  typedef std::shared_ptr<Filter> FilterPtr;
  typedef std::shared_ptr<Encode> EncodePtr;
  typedef std::shared_ptr<Output> OutputPtr;

  #define AVKID_H264_NAL_UNIT_TYPE_MASK      0x1F
  #define AVKID_H264_NAL_UNIT_TYPE_SLICE     0x01
  #define AVKID_H264_NAL_UNIT_TYPE_IDR_SLICE 0x05
  #define AVKID_H264_NAL_UNIT_TYPE_SEI       0x06
  #define AVKID_H264_NAL_UNIT_TYPE_SPS       0x07
  #define AVKID_H264_NAL_UNIT_TYPE_PPS       0x08
  #define AVKID_H264_NAL_UNIT_TYPE_AUD       0x09

}

