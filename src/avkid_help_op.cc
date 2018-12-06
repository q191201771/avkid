#include "avkid_help_op.h"
#include "avkid.hpp"

namespace avkid {

void HelpOP::global_init_ffmpeg() {
  av_log_set_level(AV_LOG_DEBUG);

  av_register_all();
  avformat_network_init();
  avfilter_register_all();
}

void HelpOP::global_deinit_ffmpeg() {
  avformat_network_deinit();
}

std::string HelpOP::av_make_error_string(int errnum) {
  char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
  av_strerror(errnum, buf, AV_ERROR_MAX_STRING_SIZE);
  return std::string(buf);
}

std::string HelpOP::stringify_ffmpeg_error(int err) {
  std::ostringstream ss;
  //ss << "(" << err << ":" << av_err2str(err) << ")";
  ss << "(" << err << ":" << av_make_error_string(err) << ")";
  return ss.str();
}

void HelpOP::deserialize_from_extradata(uint8_t *extradata,
                                       int extradata_size,
                                       unsigned short *sps_len /*out*/,
                                       uint8_t **sps_data /*out*/,
                                       unsigned short *pps_len /*out*/,
                                       uint8_t **pps_data /*out*/)
{
    *sps_len  = ntohs(*(unsigned short *)(extradata + 6));
    *sps_data = extradata + 6 + sizeof(*sps_len);
    *pps_len  = ntohs(*(unsigned short *)(extradata + 6 + sizeof(*sps_len) + *sps_len + 1));
    *pps_data = extradata + 6 + sizeof(*sps_len) + *sps_len + 1 + sizeof(*pps_len);
}

void HelpOP::serialize_to_extradata(unsigned short sps_len,
                                   uint8_t *sps_data,
                                   unsigned short pps_len,
                                   uint8_t *pps_data,
                                   uint8_t *extradata /*out*/,
                                   int *extradata_size /*out*/)
{
  int i = 0;
  uint8_t *p = extradata;
  p[i++] = 0x01;
  p[i++] = sps_data[1];
  p[i++] = sps_data[2];
  p[i++] = sps_data[3];
  p[i++] = 0xff;
  p[i++] = 0xe1;

  p[i++] = (sps_len >> 8) & 0xff;
  p[i++] = sps_len & 0xff;
  memcpy(&p[i], sps_data, sps_len);
  i += sps_len;

  p[i++] = 0x01;
  p[i++] = (pps_len >> 8) & 0xff;
  p[i++] = pps_len & 0xff;
  memcpy(&p[i], pps_data, pps_len);
  i += pps_len;

  *extradata_size = i;
}

AVFrame *HelpOP::scale_video_frame(AVFrame *frame, int width, int height) {
  int iret = -1;
  AVFrame *dst_frame = av_frame_alloc();
  dst_frame->width = width;
  dst_frame->height = height;
  int n = av_image_get_buffer_size((enum AVPixelFormat)frame->format, width, height, 1);
  uint8_t *buf = (uint8_t *)av_malloc(n * sizeof(uint8_t));
  AVKID_LOG_DEBUG << "wh:" << dst_frame->width << " " << dst_frame->height << "\n";

  if ((iret = av_image_fill_arrays(dst_frame->data, dst_frame->linesize, buf, (enum AVPixelFormat)frame->format, width, height, 1)) < 0) {
    AVKID_LOG_ERROR << "\n";
    return nullptr;
  }
  AVKID_LOG_DEBUG << "wh:" << dst_frame->width << " " << dst_frame->height << "\n";

  SwsContext *sws_ctx = sws_getContext(frame->width, frame->height, (enum AVPixelFormat)frame->format,
                                       width, height, (enum AVPixelFormat)frame->format,
                                       SWS_BICUBIC, nullptr, nullptr, nullptr);

  sws_scale(sws_ctx, (const uint8_t * const*)frame->data, frame->linesize, 0, frame->height, dst_frame->data, dst_frame->linesize);
  sws_freeContext(sws_ctx);

  return dst_frame;
}

bool HelpOP::dump_mjpeg(AVFrame *frame, const std::string &filename) {
  int ret;
  bool bret = false;

  AVFormatContext *fmt_ctx = nullptr;
  AVCodec *codec = nullptr;
  AVStream *stream = nullptr;
  AVCodecContext *cc = nullptr;
  AVPacket packet;
  AVCodecID codec_id = AV_CODEC_ID_MJPEG;

  avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, filename.c_str());
  if (!fmt_ctx) {
    AVKID_LOG_ERROR << "Func avformat_alloc_output_context2 failed.\n";
    goto END;
  }

  fmt_ctx->oformat->video_codec = codec_id;

  codec = avcodec_find_encoder(codec_id);
  if (!codec) {
    AVKID_LOG_ERROR << "\n";
    goto END;
  }

  stream = avformat_new_stream(fmt_ctx, nullptr);
  if (!stream) {
    AVKID_LOG_ERROR << "\n";
    goto END;
  }

  stream->id = 0;

  cc = avcodec_alloc_context3(codec);
  if (!cc) {
    AVKID_LOG_ERROR << "\n";
    goto END;
  }

  cc->codec_id = codec_id;
  cc->codec_type = AVMEDIA_TYPE_VIDEO;
  cc->pix_fmt = AV_PIX_FMT_YUVJ420P;
  cc->width = frame->width;
  cc->height = frame->height;
  cc->flags |= CODEC_FLAG_QSCALE;
  cc->global_quality = FF_QP2LAMBDA * 1;

  stream->time_base = (AVRational){1, 25};
  cc->time_base = stream->time_base;

  ret = avcodec_open2(cc, codec, nullptr);
  if (ret < 0) {
    FFMPEG_FAILED_LOG("avcodec_open2", ret);
    goto END;
  }

  ret = avcodec_parameters_from_context(stream->codecpar, cc);
  if (ret < 0) {
    FFMPEG_FAILED_LOG("avcodec_parameters_from_context", ret);
    goto END;
  }

  ret = avformat_write_header(fmt_ctx, nullptr);
  if (ret < 0) {
    FFMPEG_FAILED_LOG("avformat_write_header", ret);
    goto END;
  }

  av_new_packet(&packet, cc->width * cc->height * 4);

  ret = avcodec_send_frame(cc, frame);
  if (ret < 0) {
    FFMPEG_FAILED_LOG("avcodec_send_frame", ret);
    goto END;
  }

  while (ret >= 0) {
    ret = avcodec_receive_packet(cc, &packet);
    if (ret < 0) {
      if (ret != AVERROR(EAGAIN)) {
        FFMPEG_FAILED_LOG("avcodec_receive_frame", ret);
      }
      goto END;
    }
    ret = av_interleaved_write_frame(fmt_ctx, &packet);
    if (ret < 0) {
      FFMPEG_FAILED_LOG("av_interleaved_write_frame", ret);
      goto END;
    }
  }

  av_write_trailer(fmt_ctx);

  bret = true;
  goto END;

END:
  if (cc) { avcodec_close(cc); }
  if (fmt_ctx) { avformat_free_context(fmt_ctx); }

  return bret;
}

bool HelpOP::mix_video_pin_frame(AVFrame *bg, AVFrame *part, int x, int y) {
  int bgw = bg->width;
  int bgh = bg->height;
  int ptw = part->width;
  int pth = part->height;
  if (x < 0 || y < 0 || (x+ptw) > bgw || (y+pth) > bgh) {
    AVKID_LOG_ERROR << "\n";
    return false;
  }

  for (int i = 0; i < pth; i++) {
    memcpy(bg->data[0] + bg->linesize[0]*(y+i) + x,
           part->data[0] + part->linesize[0]*i,
           ptw);
  }

  for (int i = 0; i < pth/2; i++) {
    memcpy(bg->data[1] + bg->linesize[1]*(y/2+i) + x/2,
           part->data[1] + part->linesize[1]*i,
           ptw/2);

    memcpy(bg->data[2] + bg->linesize[2]*(y/2+i) + x/2,
           part->data[2] + part->linesize[2]*i,
           ptw/2);
  }

  return true;
}

class OpenTimeoutHook {
  public:
    OpenTimeoutHook(const std::string &url) : url_(url) {}

    void call_me_before_open(AVFormatContext *fmt_ctx, uint64_t timeout_msec) {
      timeout_msec_ = timeout_msec;
      open_tick_msec_ = chef::stuff_op::tick_msec();
      fmt_ctx->interrupt_callback.callback = OpenTimeoutHook::interrupt_cb;
      fmt_ctx->interrupt_callback.opaque = (void *)this;
    }
    void call_me_after_open() {
      opened_ = true;
    }

  private:
    static int interrupt_cb(void *opaque) {
      OpenTimeoutHook *obj = (OpenTimeoutHook *)opaque;
      if (!obj->opened_ && obj->open_tick_msec_ != 0 && obj->timeout_msec_ != 0 &&
          (chef::stuff_op::tick_msec() - obj->open_tick_msec_ > obj->timeout_msec_)
      ) {
        AVKID_LOG_INFO << "Timeout. " << obj->url_ << "\n";
        return 1;
      }
      return 0;
    }

  private:
    std::string url_;
    uint64_t open_tick_msec_ = 0;
    uint64_t timeout_msec_ = 0;
    bool opened_ = false;
};

int HelpOP::open_fmt_ctx_with_timtout(AVFormatContext **fmt_ctx, const std::string &url, uint32_t timeout_msec) {
  int iret = -1;
  *fmt_ctx = avformat_alloc_context();
  // @TODO timeout
  //OpenTimeoutHook oth(url);
  //oth.call_me_before_open(*fmt_ctx, timeout_msec);
  if ((iret = avformat_open_input(fmt_ctx, url.c_str(), nullptr, nullptr)) < 0) {
    // @NOTICE 错误值举例
    // -875574520, AVERROR_HTTP_NOT_FOUND, Server returned 404 Not Found, rtmp流不存在
    // -2, , No such file or directory, 文件不存在
    // -1414092869, , Immediate exit requested, 被interrupt_cb干掉了
    return iret;
  }
  //oth.call_me_after_open();
  return iret;
}

int HelpOP::open_codec_context(int *stream_idx /*out*/,
                              AVCodecContext **codec_ctx /*out*/,
                              AVFormatContext *fmt_ctx,
                              enum AVMediaType type,
                              bool is_decode,
                              int width,
                              int height)
{
  int ret, stream_index;
  AVStream *st;
  AVCodec *codec = nullptr;
  AVDictionary *opts = nullptr;

  if ((ret = av_find_best_stream(fmt_ctx, type, -1, -1, nullptr, 0)) < 0) {
    FFMPEG_FAILED_LOG("av_find_best_stream", ret);
    return ret;
  }

  stream_index = ret;
  st = fmt_ctx->streams[stream_index];

  // 28 AV_CODEC_ID_H264
  // 86018 AV_CODEC_ID_AAC
  if (is_decode) {
    codec = avcodec_find_decoder(st->codecpar->codec_id);
  } else {
    codec = avcodec_find_encoder(st->codecpar->codec_id);
  }

  if (!codec) {
    return AVERROR(EINVAL);
  }

  *codec_ctx = avcodec_alloc_context3(codec);
  if (!*codec_ctx) {
    return AVERROR(ENOMEM);
  }

  if ((ret = avcodec_parameters_to_context(*codec_ctx, st->codecpar)) < 0) {
    FFMPEG_FAILED_LOG("avcodec_parameters_to_context", ret);
    return ret;
  }

  if (!is_decode) {
    (*codec_ctx)->time_base = st->time_base;

    if (type == AVMEDIA_TYPE_VIDEO) {
      if (width != -1 && height != -1) {
        (*codec_ctx)->width = width;
        (*codec_ctx)->height = height;
      }
    }
  }

  if ((ret = avcodec_open2(*codec_ctx, codec, &opts)) < 0) {
    FFMPEG_FAILED_LOG("avcodec_open2", ret);
    return ret;
  }

  *stream_idx = stream_index;

  return 0;
}

AVFrame *HelpOP::share_frame(AVFrame *frame) {
  return frame ? av_frame_clone(frame) : nullptr;
}

AVPacket *HelpOP::share_packet(AVPacket *packet) {
  return packet ? av_packet_clone(packet) : nullptr;
}

void HelpOP::unshare_frame(AVFrame *frame) {
  if (frame) { av_frame_unref(frame); }
}

void HelpOP::unshare_packet(AVPacket *packet) {
  if (packet) { av_packet_unref(packet); }
}

}
