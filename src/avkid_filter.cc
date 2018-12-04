#include "avkid_filter.h"

namespace avkid {

bool Filter::open(AVFormatContext *in_fmt_ctx) {
  int iret = -1;

  // TODO to interface param
  //const char *filter_descr = "drawtext=\"text='Test Text'\"";
  //const char *filter_descr = "boxblur=2:1:cr=0:ar=0";
  // 画一个框
  //const char *filter_descr = "drawbox=x=10:y=20:w=200:h=60:color=red@0.5";
  // 控制yuv值，类似黑白效果？
  const char *filter_descr = "lutyuv='u=128:v=128'";
  // 左右翻转
  //const char *filter_descr = "hflip";
  // 上下翻转
  //const char *filter_descr = "vflip";

  bool has_video = false;
  int width, height;
  AVPixelFormat pix_fmt;
  AVRational time_base;
  AVRational sar = {0, 1}; // TODO
  for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
    AVStream *in_stream = in_fmt_ctx->streams[i];
    AVCodecParameters *codecpar = in_stream->codecpar;

    if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      width = codecpar->width;
      height = codecpar->height;
      pix_fmt = AV_PIX_FMT_YUV420P; // TODO
      time_base = in_stream->time_base;
      //sar = codecpar->sample_aspect_ratio;
      has_video = true;
      break;
    }
  }

  if (!has_video) { return false; }

  const AVFilter *buffersrc = avfilter_get_by_name("buffer");
  const AVFilter *buffersink = avfilter_get_by_name("buffersink");
  AVFilterInOut *outputs = avfilter_inout_alloc();
  AVFilterInOut *inputs = avfilter_inout_alloc();
  AVFilterGraph *filter_graph = avfilter_graph_alloc();
  if (!outputs || !inputs || !filter_graph) { return false; }

  char args[512] = {0};
  snprintf(args, sizeof(args)-1,
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           width, height, pix_fmt, time_base.num, time_base.den, sar.num, sar.den);
  AVKID_LOG_DEBUG << args << "\n";

  if ((iret = avfilter_graph_create_filter(&buffersrc_ctx_, buffersrc, "in", args, nullptr, filter_graph)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  if ((iret = avfilter_graph_create_filter(&buffersink_ctx_, buffersink, "out", nullptr, nullptr, filter_graph)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  AVPixelFormat pix_fmts[] = { pix_fmt, AV_PIX_FMT_NONE };
  if ((iret = av_opt_set_int_list(buffersink_ctx_, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  outputs->name = av_strdup("in");
  outputs->filter_ctx = buffersrc_ctx_;
  outputs->pad_idx = 0;
  outputs->next = nullptr;

  inputs->name = av_strdup("out");
  inputs->filter_ctx = buffersink_ctx_;
  inputs->pad_idx = 0;
  inputs->next = nullptr;

  if ((iret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, nullptr)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  if ((iret = avfilter_graph_config(filter_graph, nullptr)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return false;
  }

  // TODO
  avfilter_inout_free(&inputs);
  avfilter_inout_free(&outputs);
  return true;
}

void Filter::do_frame(AVFrame *frame, bool is_audio) {
  int iret = -1;

  if (is_audio) {
    if (fh_) { fh_(frame, is_audio); }

    return;
  }

  AVFrame *filt_frame = av_frame_alloc();


  //frame->pts = frame->best_effort_timestamp;

  if ((iret = av_buffersrc_add_frame_flags(buffersrc_ctx_, frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
    AVKID_LOG_FFMPEG_ERROR(iret);
    return;
  }

  while (1) {
    iret = av_buffersink_get_frame(buffersink_ctx_, filt_frame);
    if (iret == AVERROR(EAGAIN) || iret == AVERROR_EOF) { break; }

    if (iret < 0) {
      AVKID_LOG_FFMPEG_ERROR(iret);
      break;
    }

    if (fh_) { fh_(filt_frame, is_audio); }

    av_frame_unref(filt_frame);
  }
}

}
