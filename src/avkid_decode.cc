#include "avkid_decode.h"

namespace avkid {

Decode::Decode(DecodeObserver *obs, bool async_mode)
  : obs_(obs)
  , async_mode_(async_mode)
{
  if (async_mode) {
    thread_ = std::make_shared<chef::task_thread>("avkid_decode", chef::task_thread::RELEASE_MODE_DO_ALL_DONE);
    thread_->start();
  }
}

Decode::~Decode() {
  avcodec_free_context(&audio_dec_ctx_);
  avcodec_free_context(&video_dec_ctx_);
}

bool Decode::open(AVFormatContext *in_fmt_ctx) {
  int iret = -1;
  int audio_stream_index = -1;
  int video_stream_index = -1;
  if ((iret = __open_codec_context(&audio_stream_index, &audio_dec_ctx_, in_fmt_ctx, AVMEDIA_TYPE_AUDIO)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }
  if ((iret = __open_codec_context(&video_stream_index, &video_dec_ctx_, in_fmt_ctx, AVMEDIA_TYPE_VIDEO)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  return true;
}

bool Decode::do_packet_(AVPacket *pkt, bool is_audio) {
  AVFrame *frame = av_frame_alloc();

  int iret = -1;
  AVCodecContext *dec_ctx = is_audio ? audio_dec_ctx_ : video_dec_ctx_;
  if ((iret = avcodec_send_packet(dec_ctx, pkt)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    goto END;
  }

  if ((iret = avcodec_receive_frame(dec_ctx, frame)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    goto END;
  }

  if (obs_) { obs_->frame_cb(frame, is_audio); }

END:
  av_packet_unref(pkt);
  av_frame_unref(frame);

  return (iret != -1);
}
bool Decode::do_packet(AVPacket *pkt, bool is_audio) {
  AVPacket *rpkt = av_packet_clone(pkt);
  if (async_mode_) {
    thread_->add(chef::bind(&Decode::do_packet_, this, rpkt, is_audio));
    return true;
  }

  return do_packet_(rpkt, is_audio);
}

void Decode::packet_cb(AVPacket *pkt, bool is_audio) {
  do_packet(pkt, is_audio);
}

}
