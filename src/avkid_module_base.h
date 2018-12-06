/**
 * @file   avkid_base.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

enum audio_video_flag {
  avf_none  = 0,
  avf_audio = 1,
  avf_video = 2,
  avf_both  = 4
};

class ModuleBase {
  public:
    virtual ~ModuleBase() {}

    ModuleBase(bool async_mode=false);

    bool avf_audio_on() { return avf_ == avf_audio || avf_ == avf_both; }
    bool avf_video_on() { return avf_ == avf_video || avf_ == avf_both; }

  protected:
    bool async_mode_ = false;
    std::shared_ptr<chef::task_thread> thread_;
    audio_video_flag avf_ = avf_both;

};

// TODO consumer

class PacketProducer {
  public:
    virtual ~PacketProducer() {}

    void set_data_handler(PacketHandlerT ph);

  protected:
    PacketHandlerT ph_;
};

class FrameProducer {
  public:
    virtual ~FrameProducer() {}

    void set_data_handler(FrameHandlerT fh);

  protected:
    FrameHandlerT fh_;
};


}
