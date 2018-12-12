#include "avkid_module_encode.h"
#include "avkid.hpp"

namespace avkid {

std::shared_ptr<Encode> Encode::create(bool async_mode, enum AudioVideoFlag avf) {
  return std::make_shared<Encode>(async_mode, avf);
}

Encode::Encode(bool async_mode, enum AudioVideoFlag avf)
  : ModuleBase(async_mode, avf)
{
}

Encode::~Encode() {
  do_data(nullptr, true);
  do_data(nullptr, false);
  thread_.reset();
  if (audio_enc_ctx_) { avcodec_free_context(&audio_enc_ctx_); }
  if (video_enc_ctx_) { avcodec_free_context(&video_enc_ctx_); }
}

bool Encode::open(AVFormatContext *in_fmt_ctx, int width, int height) {
  int iret = -1;
  int audio_stream_index = -1;
  int video_stream_index = -1;
  if (avf_audio_on()) {
    if ((iret = HelpOP::open_codec_context(&audio_stream_index, &audio_enc_ctx_, in_fmt_ctx, AVMEDIA_TYPE_AUDIO, false)) < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return false;
    }
  }
  if (avf_video_on()) {
    if ((iret = HelpOP::open_codec_context(&video_stream_index, &video_enc_ctx_, in_fmt_ctx, AVMEDIA_TYPE_VIDEO, false, width, height)) < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return false;
    }
  }

  return true;
}

void Encode::do_data(AVFrame *frame, bool is_audio) {
  if (is_audio && !avf_audio_on()) { return; }
  if (!is_audio && !avf_video_on()) { return; }

  AVFrame *rframe = HelpOP::frame_alloc_copy_prop_ref_buf(frame);
  if (async_mode_) {
    thread_->add(chef::bind(&Encode::do_frame_, this, rframe, is_audio));
    return;
  }
  do_frame_(rframe, is_audio);
}

bool Encode::do_frame_(AVFrame *frame, bool is_audio) {
  if (is_audio) {
    do_audio_frame(frame);
  } else {
    do_video_frame(frame);
  }
  HelpOP::frame_free_prop_unref_buf(&frame);
  // TODO
  return true;
}

void Encode::do_audio_frame(AVFrame *frame) {
  int iret = -1;
  AVPacket packet = {0};

  //AVKID_LOG_FRAME(frame, true);

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

    //AVKID_LOG_PACKET(&packet, true);
    if (packet_handler) { packet_handler(&packet, true); }
  }
}

void Encode::do_video_frame(AVFrame *frame) {
  int iret = -1;
  AVPacket packet = {0};

  //AVKID_LOG_FRAME(frame, false);

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

    //AVKID_LOG_PACKET(&packet, false);
    if (packet_handler) { packet_handler(&packet, false); }
  }
}

}
