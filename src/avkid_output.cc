#include "avkid_output.h"

namespace avkid {

Output::Output(bool async_mode)
  : async_mode_(async_mode)
{
  if (async_mode) {
    thread_ = std::make_shared<chef::task_thread>("avkid_ouput", chef::task_thread::RELEASE_MODE_DO_ALL_DONE);
    thread_->start();
  }
}

Output::~Output() {
  if (out_fmt_ctx_) {
    av_write_trailer(out_fmt_ctx_);
    if (out_fmt_ctx_ && !(out_fmt_ctx_->oformat->flags & AVFMT_NOFILE))
      avio_closep(&out_fmt_ctx_->pb);
    avformat_free_context(out_fmt_ctx_);
  }
}

bool Output::open(const std::string &url, AVFormatContext *in_fmt_ctx) {
  int iret = -1;

  avformat_alloc_output_context2(&out_fmt_ctx_, nullptr, nullptr, url.c_str());
  if (!out_fmt_ctx_) { return false; }

  int stream_index = 0;
  for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
    AVStream *out_stream = nullptr;
    AVStream *in_stream = in_fmt_ctx->streams[i];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    switch (in_codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO: audio_stream_index_ = stream_index++; break;
    case AVMEDIA_TYPE_VIDEO: video_stream_index_ = stream_index++; break;
    default: continue;
    }

    out_stream = avformat_new_stream(out_fmt_ctx_, nullptr);
    if (!out_stream) { return false; }

    if ((iret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar)) < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return false;
    }

    out_stream->codecpar->codec_tag = 0;
  }

  av_dump_format(out_fmt_ctx_, 0, url.c_str(), 1);

  if (!(out_fmt_ctx_->flags & AVFMT_NOFILE)) {
    if ((iret = avio_open(&out_fmt_ctx_->pb, url.c_str(), AVIO_FLAG_WRITE)) < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return false;
    }
  }

  if ((iret = avformat_write_header(out_fmt_ctx_, nullptr)) < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      return false;
  }

  return true;
}

bool Output::do_packet_(AVPacket *pkt, bool is_audio) {
  int iret = -1;
  if ((iret = av_interleaved_write_frame(out_fmt_ctx_, pkt)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  av_packet_unref(pkt);
  return true;
}

bool Output::do_packet(AVPacket *pkt, bool is_audio) {
  if ((is_audio && audio_stream_index_ == -1) ||
      (!is_audio && video_stream_index_ == -1)
  ) {
    return false;
  }

  AVPacket *rpkt = av_packet_clone(pkt);
  if (async_mode_) {
    thread_->add(chef::bind(&Output::do_packet_, this, rpkt, is_audio));
    return true;
  }

  return do_packet_(rpkt, is_audio);
}

void Output::packet_cb(AVPacket *pkt, bool is_audio) {
  do_packet(pkt, is_audio);
}

}
