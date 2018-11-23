#include "avkid_input.h"

namespace avkid {

Input::~Input() {
  avformat_close_input(&in_fmt_ctx_);
}

bool Input::open(const std::string &url, uint32_t timeout_msec) {
  int iret = -1;
  url_ = url;

  if ((iret = __open_fmt_ctx_with_timtout(&in_fmt_ctx_, url, timeout_msec)) < 0) {
    goto END;
  }

  if ((iret = avformat_find_stream_info(in_fmt_ctx_, nullptr)) < 0) {
    goto END;
  }

  av_dump_format(in_fmt_ctx_, 0, url.c_str(), 0);

END:
  if (iret < 0) {
    AVKID_LOG_ERROR << stringify_ffmpeg_error(iret) << "\n";
  }
  return iret >= 0;
}

bool Input::read() {
  int iret = -1;

  AVPacket pkt = {0};

  while (!stop_read_flag_ && av_read_frame(in_fmt_ctx_, &pkt) >= 0) {
    if (obs_) {
      int ct = in_fmt_ctx_->streams[pkt.stream_index]->codecpar->codec_type;
      if (ct != AVMEDIA_TYPE_AUDIO && ct != AVMEDIA_TYPE_VIDEO) {
        AVKID_LOG_ERROR << "Unknown codec type:" << ct << "\n";
        continue;
      }
      obs_->packet_cb(&pkt, ct == AVMEDIA_TYPE_AUDIO);
    }

    av_packet_unref(&pkt);
  }

  return true;
}

}
