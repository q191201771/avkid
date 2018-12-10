/**
 * @file   avkid_base.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

enum AudioVideoFlag {
  AVF_NONE  = 0,
  AVF_AUDIO = 1,
  AVF_VIDEO = 2,
  AVF_BOTH  = 4
};

class ModuleBase {
  public:
    virtual ~ModuleBase() {}

    ModuleBase(bool async_mode=false, enum AudioVideoFlag avf=AVF_BOTH);

    bool avf_audio_on() { return avf_ == AVF_AUDIO || avf_ == AVF_BOTH; }
    bool avf_video_on() { return avf_ == AVF_VIDEO || avf_ == AVF_BOTH; }

  protected:
    bool async_mode_ = false;
    std::shared_ptr<chef::task_thread> thread_;
    AudioVideoFlag avf_ = AVF_BOTH;

};

class PacketProducer {
  public:
    virtual ~PacketProducer() {}

    void set_data_handler(PacketHandlerT ph);

  protected:
    PacketHandlerT packet_handler;
};

class FrameProducer {
  public:
    virtual ~FrameProducer() {}

    void set_data_handler(FrameHandlerT fh);

  protected:
    FrameHandlerT frame_handler;
};

}
