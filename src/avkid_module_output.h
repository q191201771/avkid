/**
 * @file   avkid_output.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class Output {
  public:
    Output(bool async_mode=false);
    ~Output();
    static std::shared_ptr<Output> create(bool async_mode=false);

    // TODO timeout
    bool open(const std::string &url, AVFormatContext *in_fmt_ctx, int width=-1, int height=-1);

    bool do_data(AVPacket *pkt, bool is_audio);

  private:
    bool do_packet_(AVPacket *pkt, bool is_audio);

  private:
    bool async_mode_ = false;
    AVFormatContext *out_fmt_ctx_ = nullptr;
    int audio_stream_index_ = -1;
    int video_stream_index_ = -1;
    std::shared_ptr<chef::task_thread> thread_;

  private:
    Output(const Output &) = delete;
    Output &operator=(const Output &) = delete;

};

}
