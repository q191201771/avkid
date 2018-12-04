#include "avkid_decode.h"

namespace avkid {

Decode::Decode(FrameHandlerT fh, bool async_mode)
  : fh_(fh)
  , async_mode_(async_mode)
{
  if (async_mode) {
    thread_ = std::make_shared<chef::task_thread>("avkid_decode", chef::task_thread::RELEASE_MODE_DO_ALL_DONE);
    thread_->start();
  }
}

Decode::~Decode() {
  do_packet(nullptr, true);
  do_packet(nullptr, false);
  thread_.reset();
  avcodec_free_context(&audio_dec_ctx_);
  avcodec_free_context(&video_dec_ctx_);
}

bool Decode::open(AVFormatContext *in_fmt_ctx) {
  int iret = -1;
  int audio_stream_index = -1;
  int video_stream_index = -1;
  if ((iret = __open_codec_context(&audio_stream_index, &audio_dec_ctx_, in_fmt_ctx, AVMEDIA_TYPE_AUDIO, true)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }
  if ((iret = __open_codec_context(&video_stream_index, &video_dec_ctx_, in_fmt_ctx, AVMEDIA_TYPE_VIDEO, true)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  return true;
}

bool Decode::do_packet_(AVPacket *pkt, bool is_audio) {
  AVFrame *frame = av_frame_alloc();

  int iret = -1;
  AVCodecContext *dec_ctx = is_audio ? audio_dec_ctx_ : video_dec_ctx_;

  //AVKID_LOG_PACKET(pkt, is_audio);

  if ((iret = avcodec_send_packet(dec_ctx, pkt)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    goto END;
  }

  while (iret >= 0) {
    iret = avcodec_receive_frame(dec_ctx, frame);
    if (iret == AVERROR(EAGAIN) || iret == AVERROR_EOF) {
      iret = 0;
      goto END;
    } else if (iret < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      goto END;
    }

    //AVKID_LOG_FRAME(frame, is_audio);
    if (fh_) { fh_(frame, is_audio); }
  }

END:
  if (pkt) { av_packet_unref(pkt); }

  av_frame_unref(frame);

  return (iret != -1);
}
bool Decode::do_packet(AVPacket *pkt, bool is_audio) {
  AVPacket *rpkt = pkt ? av_packet_clone(pkt) : pkt;
  if (async_mode_) {
    thread_->add(chef::bind(&Decode::do_packet_, this, rpkt, is_audio));
    return true;
  }

  return do_packet_(rpkt, is_audio);
}

}
