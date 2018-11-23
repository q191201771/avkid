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

    bool read();

    void stop_read() { stop_read_flag_ = true; }

  public:
    CHEF_PROPERTY_WITH_INIT_VALUE(AVFormatContext *, in_fmt_ctx, nullptr);

  private:
    PacketHandler *ph_ = nullptr;
    std::string url_;
    bool stop_read_flag_ = false;

  private:
    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;

};

}
