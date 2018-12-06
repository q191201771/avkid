#include "avkid_module_input.h"
#include "avkid.hpp"

namespace avkid {

std::shared_ptr<Input> Input::create() {
  return std::make_shared<Input>();
}

Input::Input() {}

Input::~Input() {
  avformat_close_input(&in_fmt_ctx_);
}

bool Input::open(const std::string &url, uint32_t timeout_msec, enum audio_video_flag avf) {
  int iret = -1;
  url_ = url;
  avf_ = avf;

  if ((iret = HelpOP::open_fmt_ctx_with_timtout(&in_fmt_ctx_, url, timeout_msec)) < 0) {
    goto END;
  }

  if ((iret = avformat_find_stream_info(in_fmt_ctx_, nullptr)) < 0) {
    goto END;
  }

  av_dump_format(in_fmt_ctx_, 0, url.c_str(), 0);

  for (int i = 0; i < in_fmt_ctx_->nb_streams; i++) {
    AVStream *stream = in_fmt_ctx_->streams[i];
    AVCodecParameters *codecpar = stream->codecpar;

    if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_width_ = codecpar->width;
      video_height_ = codecpar->height;
    }
  }

END:
  if (iret < 0) { AVKID_LOG_FFMPEG_ERROR(iret); }

  return iret >= 0;
}

bool Input::read(uint32_t duration_ms) {
  int iret = -1;

  AVPacket pkt = {0};

  while (!stop_read_flag_ && av_read_frame(in_fmt_ctx_, &pkt) >= 0) {
    int ct = in_fmt_ctx_->streams[pkt.stream_index]->codecpar->codec_type;

    if (ct == AVMEDIA_TYPE_AUDIO) {
      if (first_audio_pts_ == -1) { first_audio_pts_ = pkt.pts; }
      if (first_audio_dts_ == -1) { first_audio_dts_ = pkt.dts; }

      audio_duration_ = pkt.pts - first_audio_pts_;
      pkt.pts -= first_audio_pts_;
      pkt.dts -= first_audio_dts_;
    } else {
      if (first_video_pts_ == -1) { first_video_pts_ = pkt.pts; }
      if (first_video_dts_ == -1) { first_video_dts_ = pkt.dts; }

      video_duration_ = pkt.pts - first_video_pts_;
      pkt.pts -= first_video_pts_;
      pkt.dts -= first_video_dts_;
    }

    if (duration_ms != 0 && audio_duration_ > duration_ms && video_duration_ > duration_ms) {
      stop_read_flag_ = true;
    }

    if (ph_) {
      if (ct == AVMEDIA_TYPE_AUDIO) {
        if (avf_audio_on()) { ph_(&pkt, true); }
      } else if (ct == AVMEDIA_TYPE_VIDEO) {
        if (avf_video_on()) { ph_(&pkt, false); }
      } else {
        AVKID_LOG_ERROR << "Unknown codec type:" << ct << "\n";
      }
    }

    HelpOP::packet_unref_buf(&pkt);
  }

  return true;
}

}
