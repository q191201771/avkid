/**
 * @file   avkid_module_decode.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"
#include "avkid_module_base.h"

namespace avkid {

class Decode : public ModuleBase, public FrameProducer {
  public:
    Decode(bool async_mode=false);
    ~Decode();
    static std::shared_ptr<Decode> create(bool async_mode=false);

    bool open(AVFormatContext *in_fmt_ctx);

    bool do_data(AVPacket *pkt, bool is_audio);

  private:
    bool do_packet_(AVPacket *pkt, bool is_audio);

  private:
    AVCodecContext *audio_dec_ctx_ = nullptr;
    AVCodecContext *video_dec_ctx_ = nullptr;

  private:
    Decode(const Decode &) = delete;
    Decode &operator=(const Decode &) = delete;

};

}
