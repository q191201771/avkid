/**
 * @file   avkid_filter.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class Filter {
  public:
    Filter(bool async_mode=false);
    ~Filter();

    void set_frame_handler(FrameHandlerT fh);

    bool open(AVFormatContext *in_fmt_ctx);

    void do_frame(AVFrame *frame, bool is_audio);

  private:
    void do_frame_(AVFrame *frame, bool is_audio);

  private:
    FrameHandlerT fh_;
    bool async_mode_ = false;
    std::shared_ptr<chef::task_thread> thread_;

    AVFilterContext *buffersrc_ctx_ = nullptr;
    AVFilterContext *buffersink_ctx_ = nullptr;

  private:
    Filter(const Filter &) = delete;
    Filter &operator=(const Filter &) = delete;

};

}
