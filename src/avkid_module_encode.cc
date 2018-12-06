#include "avkid_module_encode.h"
#include "avkid.hpp"

namespace avkid {

std::shared_ptr<Encode> Encode::create(bool async_mode) {
  return std::make_shared<Encode>(async_mode);
}

Encode::Encode(bool async_mode)
  : async_mode_(async_mode)
{
  if (async_mode) {
    thread_ = std::make_shared<chef::task_thread>("avkid_encode", chef::task_thread::RELEASE_MODE_DO_ALL_DONE);
    thread_->start();
  }
}

Encode::~Encode() {
  do_data(nullptr, true);
  do_data(nullptr, false);
  thread_.reset();
}

bool Encode::open(AVFormatContext *in_fmt_ctx, int width, int height) {
  int iret = -1;
  int audio_stream_index = -1;
  int video_stream_index = -1;
  if ((iret = HelpOP::open_codec_context(&audio_stream_index, &audio_enc_ctx_, in_fmt_ctx, AVMEDIA_TYPE_AUDIO, false)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }
  if ((iret = HelpOP::open_codec_context(&video_stream_index, &video_enc_ctx_, in_fmt_ctx, AVMEDIA_TYPE_VIDEO, false, width, height)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  return true;
}

bool Encode::do_data(AVFrame *frame, bool is_audio) {
  AVFrame *rframe = HelpOP::share_frame(frame);
  if (async_mode_) {
    thread_->add(chef::bind(&Encode::do_frame_, this, rframe, is_audio));
    return true;
  }
  return do_frame_(rframe, is_audio);
}

bool Encode::do_frame_(AVFrame *frame, bool is_audio) {
  if (is_audio) {
    do_audio_frame(frame);
  } else {
    do_video_frame(frame);
  }
  HelpOP::unshare_frame(frame);
  // TODO
  return true;
}

void Encode::do_audio_frame(AVFrame *frame) {
  int iret = -1;
  AVPacket packet = {0};

  AVKID_LOG_FRAME(frame, true);

  if ((iret = avcodec_send_frame(audio_enc_ctx_, frame)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return;
  }

  while(iret >= 0) {
    iret = avcodec_receive_packet(audio_enc_ctx_, &packet);
    if (iret == AVERROR(EAGAIN) || iret == AVERROR_EOF) {
      return;
    } else if (iret < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return;
    }

    AVKID_LOG_PACKET(&packet, true);
    if (ph_) { ph_(&packet, true); }
  }
}

void Encode::do_video_frame(AVFrame *frame) {
  int iret = -1;
  AVPacket packet = {0};

  AVKID_LOG_FRAME(frame, false);

  if ((iret = avcodec_send_frame(video_enc_ctx_, frame)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return;
  }
  while(iret >= 0) {
    iret = avcodec_receive_packet(video_enc_ctx_, &packet);
    if (iret == AVERROR(EAGAIN) || iret == AVERROR_EOF) {
      return;
    } else if (iret < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return;
    }

    AVKID_LOG_PACKET(&packet, false);
    if (ph_) { ph_(&packet, false); }
  }
}

}
