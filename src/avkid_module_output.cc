#include "avkid_module_output.h"
#include "avkid.hpp"

namespace avkid {

std::shared_ptr<Output> Output::create(bool async_mode, enum AudioVideoFlag avf) {
  return std::make_shared<Output>(async_mode, avf);
}

Output::Output(bool async_mode, enum AudioVideoFlag avf)
  : ModuleBase(async_mode, avf)
{
}

Output::~Output() {
  thread_.reset();
  if (out_fmt_ctx_) {
    av_write_trailer(out_fmt_ctx_);

    if (out_fmt_ctx_ && !(out_fmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
      avio_closep(&out_fmt_ctx_->pb);
    }

    avformat_free_context(out_fmt_ctx_);
  }
}

bool Output::open(const std::string &url, AVFormatContext *in_fmt_ctx, int width, int height) {
  int iret = -1;

  std::string format_name;

  if ((chef::strings_op::has_prefix(url, "http://") && chef::strings_op::has_suffix(url, ".flv")) ||
      chef::strings_op::has_prefix(url, "rtmp://")
  ) {
    format_name = "flv";
  }

  avformat_alloc_output_context2(&out_fmt_ctx_, nullptr, format_name.empty() ? nullptr : format_name.c_str(), url.c_str());
  if (!out_fmt_ctx_) { return false; }

  int stream_index = 0;
  for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
    AVStream *out_stream = nullptr;
    AVStream *in_stream = in_fmt_ctx->streams[i];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    if ((avf_audio_on() && in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) ||
        (avf_video_on() && in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
    ) {
      out_stream = avformat_new_stream(out_fmt_ctx_, nullptr);
      if (!out_stream) { return false; }

      if ((iret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar)) < 0) {
        AVKID_LOG_FFMPEG_ERROR(iret);
        return false;
      }

      out_stream->codecpar->codec_tag = 0;

      if (in_codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        in_audio_stream_index_ = stream_index++;

        out_audio_stream_ = out_stream;
        in_audio_time_base_ = in_stream->time_base;
      }
      if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        in_video_stream_index_ = stream_index++;

        out_video_stream_ = out_stream;
        in_video_time_base_ = in_stream->time_base;

        if (width != -1 && height != -1) {
          out_stream->codecpar->width = width;
          out_stream->codecpar->height = height;
        }
      }
    }

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

  pkt->stream_index = is_audio ? in_audio_stream_index_ : in_video_stream_index_;
  AVStream *out_stream = is_audio ? out_audio_stream_ : out_video_stream_;
  AVRational in_time_base = is_audio ? in_audio_time_base_ : in_video_time_base_;
  AVRational out_time_base = out_stream->time_base;

  if (!is_audio && ((*(pkt->data+4) & AVKID_H264_NAL_UNIT_TYPE_MASK) == AVKID_H264_NAL_UNIT_TYPE_SEI)) {
    AVKID_LOG_INFO << "Drop SEI.\n";
    return true;
  }

  pkt->pts = av_rescale_q_rnd(pkt->pts, in_time_base, out_time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
  pkt->dts = av_rescale_q_rnd(pkt->dts, in_time_base, out_time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
  pkt->duration = av_rescale_q(pkt->duration, in_time_base, out_time_base);

  //AVKID_LOG_PACKET(pkt, is_audio);
  //AVKID_LOG_INFO << "CHEFERASEME "
  //               << av_ts2str(pkt->pts) << " " << av_ts2timestr(pkt->pts, &out_stream->time_base)
  //               << " " << av_ts2str(pkt->dts) << " " << av_ts2timestr(pkt->dts, &out_stream->time_base)
  //               << " out_stream(" << out_stream->time_base.num << " " << out_stream->time_base.den << ")"
  //               << " in_audio_tb(" << in_audio_time_base_.num << " " << in_audio_time_base_.den << ")"
  //               << " in_video_tb(" << in_video_time_base_.num << " " << in_video_time_base_.den << ")"
  //               << "\n";

  if ((iret = av_interleaved_write_frame(out_fmt_ctx_, pkt)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  HelpOP::packet_free_prop_unref_buf(&pkt);
  return true;
}

void Output::do_data(AVPacket *pkt, bool is_audio) {
  if (is_audio && in_audio_stream_index_ == -1) { return; }
  if (!is_audio && in_video_stream_index_ == -1) { return; }

  AVPacket *rpkt = HelpOP::packet_alloc_prop_ref_buf(pkt);
  if (async_mode_) {
    thread_->add(chef::bind(&Output::do_packet_, this, rpkt, is_audio));
    return;
  }

  do_packet_(rpkt, is_audio);
}

}
