/**
 * @file   avkid_filter.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"
#include "avkid_module_base.h"

namespace avkid {

class Filter : public ModuleBase, public FrameConsumerInterface, public FrameProducer {
  public:
    Filter(bool async_mode=false, enum AudioVideoFlag avf=AVF_BOTH);
    ~Filter();
    static std::shared_ptr<Filter> create(bool async_mode=false, enum AudioVideoFlag avf=AVF_BOTH);

    bool open(AVFormatContext *in_fmt_ctx);

    virtual void do_data(AVFrame *frame, bool is_audio);

  private:
    bool do_frame_(AVFrame *frame, bool is_audio);

  private:
    AVFilterContext *buffersrc_ctx_ = nullptr;
    AVFilterContext *buffersink_ctx_ = nullptr;

  private:
    Filter(const Filter &) = delete;
    Filter &operator=(const Filter &) = delete;

};

}
