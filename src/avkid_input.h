/**
 * @file   avkid_input.h
 * @author chef
 *
 */

#pragma once

#include "avkid_common.hpp"
#include "chef_snippet.hpp"

namespace avkid {

class InputObserver {
  public:
    virtual ~InputObserver() {}
    virtual void packet_cb(AVPacket *pkt, bool is_audio) = 0;
};

class Input {
  public:
    Input(InputObserver *obs) : obs_(obs) {}
    ~Input();

    bool open(const std::string &url, uint32_t timeout_msec=0);

    bool read();

    void stop_read() { stop_read_flag_ = true; }

  public:
    CHEF_PROPERTY_WITH_INIT_VALUE(AVFormatContext *, in_fmt_ctx, nullptr);

  private:
    InputObserver *obs_ = nullptr;
    std::string url_;
    bool stop_read_flag_ = false;

  private:
    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;

};

}
