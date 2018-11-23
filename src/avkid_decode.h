/**
 * @file   avkid_decode.h
 * @author chef
 *
 */

#pragma once

#include "avkid_common.hpp"
#include "avkid_input.h"
#include "chef_task_thread.hpp"

namespace avkid {

class Decode : public PacketHandler {
  public:
    Decode(FrameHandler *fh, bool async_mode);
    ~Decode();

    bool open(AVFormatContext *in_fmt_ctx);

    bool do_packet(AVPacket *pkt, bool is_audio);

  public: // PacketHandler
    virtual void packet_cb(AVPacket *pkt, bool is_audio);

  private:
    bool do_packet_(AVPacket *pkt, bool is_audio);

  private:
    FrameHandler *fh_ = nullptr;
    bool async_mode_ = false;
    std::shared_ptr<chef::task_thread> thread_;

    AVCodecContext *audio_dec_ctx_ = nullptr;
    AVCodecContext *video_dec_ctx_ = nullptr;

  private:
    Decode(const Decode &) = delete;
    Decode &operator=(const Decode &) = delete;

};

}
