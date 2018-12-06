/**
 * @file   avkid_module_decode.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class Decode : public FrameProducer {
  public:
    Decode(bool async_mode=false);
    ~Decode();
    static std::shared_ptr<Decode> create(bool async_mode=false);

    bool open(AVFormatContext *in_fmt_ctx);

    bool do_data(AVPacket *pkt, bool is_audio);

  private:
    bool do_packet_(AVPacket *pkt, bool is_audio);

  private:
    bool async_mode_ = false;
    std::shared_ptr<chef::task_thread> thread_;

    AVCodecContext *audio_dec_ctx_ = nullptr;
    AVCodecContext *video_dec_ctx_ = nullptr;

  private:
    Decode(const Decode &) = delete;
    Decode &operator=(const Decode &) = delete;

};

}
