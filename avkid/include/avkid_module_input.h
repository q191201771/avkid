/**
 * @file   avkid_module_input.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"
#include "avkid_module_base.h"

namespace avkid {

class Input : public ModuleBase, public PacketProducer {
  public:
    Input(enum AudioVideoFlag avf=AVF_BOTH);
    ~Input();
    static std::shared_ptr<Input> create(enum AudioVideoFlag avf=AVF_BOTH);

    // TODO timeout
    bool open(const std::string &url, uint32_t timeout_msec=0);

    // @return 如果全部读取完成返回true，发生错误则返回false
    bool read(uint32_t duration_ms=0);

    void stop_read() { stop_read_flag_ = true; }

  public:
    CHEF_PROPERTY_WITH_INIT_VALUE(AVFormatContext *, in_fmt_ctx, nullptr);

    CHEF_PROPERTY_WITH_INIT_VALUE(int, video_width, -1);
    CHEF_PROPERTY_WITH_INIT_VALUE(int, video_height, -1);
    CHEF_PROPERTY_WITH_INIT_VALUE(int64_t, audio_duration_ms, -1);
    CHEF_PROPERTY_WITH_INIT_VALUE(int64_t, video_duration_ms, -1);

  private:
    std::string url_;
    bool stop_read_flag_ = false;

    int64_t first_audio_pts_   = -1;
    int64_t first_video_pts_   = -1;
    //int64_t audio_duration_ms_ = -1;
    //int64_t video_duration_ms_ = -1;
    int64_t first_audio_dts_   = -1;
    int64_t first_video_dts_   = -1;

  private:
    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;

};

}
