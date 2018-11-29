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
      Encode(PacketHandler *ph, bool async_mode);
      ~Encode();

      bool open(AVFormatContext *in_fmt_ctx);

      bool do_frame(AVFrame *frame, bool is_audio);

    public: // FrameHandler
      virtual void frame_cb(AVFrame *frame, bool is_audio) { do_frame(frame, is_audio); }

    private:
      bool do_frame_(AVFrame *frame, bool is_audio);
      void do_audio_frame(AVFrame *frame);
      void do_video_frame(AVFrame *frame);

    private:
      PacketHandler *ph_ = nullptr;
      bool async_mode_ = false;
      std::shared_ptr<chef::task_thread> thread_;

      AVCodecContext *audio_enc_ctx_ = nullptr;
      AVCodecContext *video_enc_ctx_ = nullptr;

    private:
      Encode(const Encode &) = delete;
      Encode &operator=(const Encode &) = delete;

  };

}
