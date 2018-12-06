/**
 * @file   avkid_filter.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class Filter : public FrameProducer {
  public:
    Filter(bool async_mode=false);
    ~Filter();
    static std::shared_ptr<Filter> create(bool async_mode=false);

    bool open(AVFormatContext *in_fmt_ctx);

    void do_data(AVFrame *frame, bool is_audio);

  private:
    void do_frame_(AVFrame *frame, bool is_audio);

  private:
    bool async_mode_ = false;
    std::shared_ptr<chef::task_thread> thread_;

    AVFilterContext *buffersrc_ctx_ = nullptr;
    AVFilterContext *buffersink_ctx_ = nullptr;

  private:
    Filter(const Filter &) = delete;
    Filter &operator=(const Filter &) = delete;

};

}