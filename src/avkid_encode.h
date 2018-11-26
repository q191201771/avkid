/**
 * @file   avkid_decode.h
 * @author chef
 *
 */

#pragma once

#include "avkid_common.hpp"
#include "chef_task_thread.hpp"

namespace avkid {

  class Encode : public FrameHandler {
    public:
      Encode(PacketHandler *ph);

      bool open(AVFormatContext *in_fmt_ctx);

    public:
      virtual void frame_cb(AVFrame *frame, bool is_audio);

    private:
      void do_audio_frame(AVFrame *frame, bool is_audio);
      void do_video_frame(AVFrame *frame, bool is_audio);

    private:
      PacketHandler *ph_;
      AVCodecContext *audio_enc_ctx_ = nullptr;
      AVCodecContext *video_enc_ctx_ = nullptr;

  };

}
