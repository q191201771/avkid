/**
 * @file   avkid_filter.h
 * @author chef
 *
 */

#pragma once

#include "avkid_common.hpp"

namespace avkid {

class Filter {
  public:
    Filter(FrameHandlerT fh) : fh_(fh) {}

    bool open(AVFormatContext *in_fmt_ctx);

    void do_frame(AVFrame *frame, bool is_audio);

  private:
    FrameHandlerT fh_;
    AVFilterContext *buffersrc_ctx_ = nullptr;
    AVFilterContext *buffersink_ctx_ = nullptr;
};

}
