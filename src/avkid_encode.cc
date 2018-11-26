#include "avkid_encode.h"

namespace avkid {

Encode::Encode(PacketHandler *ph) : ph_(ph) {}

bool Encode::open(AVFormatContext *in_fmt_ctx) {
  int iret = -1;
  int audio_stream_index = -1;
  int video_stream_index = -1;
  if ((iret = __open_codec_context(&audio_stream_index, &audio_enc_ctx_, in_fmt_ctx, AVMEDIA_TYPE_AUDIO, false)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }
  if ((iret = __open_codec_context(&video_stream_index, &video_enc_ctx_, in_fmt_ctx, AVMEDIA_TYPE_VIDEO, false)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  return true;
}

void Encode::frame_cb(AVFrame *frame, bool is_audio) {
  AVFrame *rframe = av_frame_clone(frame);
  is_audio ? do_audio_frame(rframe, is_audio) : do_video_frame(rframe, is_audio);
  av_frame_unref(rframe);
}

void Encode::do_audio_frame(AVFrame *frame, bool is_audio) {
  int iret = -1;
  AVPacket packet = {0};

  if ((iret = avcodec_send_frame(audio_enc_ctx_, frame)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return;
  }
  if ((iret = avcodec_receive_packet(audio_enc_ctx_, &packet)) < 0) {
    if (iret != AVERROR(EAGAIN)) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return;
    }
    return;
  }

  if (ph_) { ph_->packet_cb(&packet, is_audio); }

  return;
}

void Encode::do_video_frame(AVFrame *frame, bool is_audio) {
  int iret = -1;
  AVPacket packet = {0};

  assert(avcodec_is_open(video_enc_ctx_) && av_codec_is_encoder(video_enc_ctx_->codec));

  if ((iret = avcodec_send_frame(video_enc_ctx_, frame)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return;
  }
  if ((iret = avcodec_receive_packet(video_enc_ctx_, &packet)) < 0) {
    if (iret != AVERROR(EAGAIN)) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return;
    }
    return;
  }

  if (ph_) { ph_->packet_cb(&packet, is_audio); }

  return;
}

}
