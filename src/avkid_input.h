/**
 * @file   avkid_input.h
 * @author chef
 *
 */

#pragma once

#include "avkid_common.hpp"
#include "chef_snippet.hpp"

namespace avkid {

class Input {
  public:
    Input(PacketHandler *ph) : ph_(ph) {}
    ~Input();

    bool open(const std::string &url, uint32_t timeout_msec=0);

    bool read(uint32_t duration_ms=0);

    void stop_read() { stop_read_flag_ = true; }

  public:
    CHEF_PROPERTY_WITH_INIT_VALUE(AVFormatContext *, in_fmt_ctx, nullptr);

  private:
    PacketHandler *ph_ = nullptr;
    std::string url_;
    bool stop_read_flag_ = false;
    int64_t first_audio_pts_ = -1;
    int64_t first_video_pts_ = -1;
    int64_t audio_duration_ = -1;
    int64_t video_duration_ = -1;
    int64_t first_audio_dts_ = -1;
    int64_t first_video_dts_ = -1;

  private:
    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;

};

}
