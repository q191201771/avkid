/**
 * @file   avkid_decode.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class Encode {
  public:
    Encode(bool async_mode=false);
    ~Encode();

    void set_packet_handler(PacketHandlerT ph);

    bool open(AVFormatContext *in_fmt_ctx);

    bool do_frame(AVFrame *frame, bool is_audio);

  private:
    bool do_frame_(AVFrame *frame, bool is_audio);
    void do_audio_frame(AVFrame *frame);
    void do_video_frame(AVFrame *frame);

  private:
    PacketHandlerT ph_;
    bool async_mode_ = false;
    std::shared_ptr<chef::task_thread> thread_;

    AVCodecContext *audio_enc_ctx_ = nullptr;
    AVCodecContext *video_enc_ctx_ = nullptr;

  private:
    Encode(const Encode &) = delete;
    Encode &operator=(const Encode &) = delete;

};

}
