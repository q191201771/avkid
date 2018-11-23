#include "avkid_out.h"
#include "avkid_common.hpp"
#include "avkid_log_adapter.hpp"
#include <assert.h>

namespace avkid {

static void __log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
  AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
  uint32_t calc_size = ntohl(*((uint32_t *)(pkt->data)));

  printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d size:%d buf.size:%d calc_size:%d\n",
         av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
         av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
         av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
         pkt->stream_index,
         pkt->size,
         pkt->buf->size,
         calc_size);

  //av_pkt_dump_log2(NULL, 0, pkt, 0, fmt_ctx->streams[pkt->stream_index]);
}

Out::Out(InputType it) {
  input_type_ = it;
  thread_ = new chef::task_thread("out", chef::task_thread::RELEASE_MODE_DO_ALL_DONE);
  thread_->start();
}

Out::~Out() {
  if (thread_) { delete thread_; thread_ = NULL; }

  if (sps_data_) { free(sps_data_); sps_data_ = NULL; }

  if (pps_data_) { free(pps_data_); pps_data_ = NULL; }

  if (fmt_ctx_) { avformat_close_input(&fmt_ctx_); fmt_ctx_ = NULL; }
}

int Out::interrupt_cb(void *opaque) {
  Out *obj = (Out *)opaque;
  if (obj->opened_ == false && obj->open_ms_ != 0 && obj->timeout_ms_ != 0 &&
      (chef::stuff_op::tick_msec() - obj->open_ms_ > obj->timeout_ms_)
  ) {
    AVKID_LOG_INFO << "Timeout. interrupt_cb." << "\n";
    return 1;
  }
  return 0;
}

void Out::init_audio(AVSampleFormat audio_sample_fmt, int audio_bit_rate, int audio_sample_rate,
                              int audio_channel_layout
) {
  has_audio_ = true;

  audio_sample_fmt_ = audio_sample_fmt;
  audio_bit_rate_ = audio_bit_rate;
  audio_sample_rate_ = audio_sample_rate;
  audio_channel_layout_ = audio_channel_layout;
}

void Out::init_video(AVPixelFormat pix_fmt,
                              int width,
                              int height,
                              int gop_size,
                              int64_t bit_rate,
                              int video_frame_rate,
                              unsigned short sps_len,
                              uint8_t *sps_data,
                              unsigned short pps_len,
                              uint8_t *pps_data)
{
  has_video_ = true;

  pix_fmt_  = pix_fmt;
  width_    = width;
  height_   = height;
  gop_size_ = gop_size;
  bit_rate_ = bit_rate;
  video_frame_rate_ = video_frame_rate;

  sps_len_ = sps_len;
  sps_data_ = (uint8_t *)malloc(sps_len);
  memcpy(sps_data_, sps_data, sps_len);
  pps_len_ = pps_len;
  pps_data_ = (uint8_t *)malloc(pps_len);
  memcpy(pps_data_, pps_data, pps_len);
}

bool Out::add_audio_stream() {
  AVCodecID codec_id = AV_CODEC_ID_AAC;
  // AVCodecID codec_id = AV_CODEC_ID_MP3;

  audio_codec_ = avcodec_find_encoder(codec_id);
  // audio_codec_ = avcodec_find_encoder_by_name("libfdk_aac");
  RETURN_FALSE_IF_NULL(audio_codec_, "avcodec_find_encoder");

  audio_stream_ = avformat_new_stream(fmt_ctx_, NULL);
  RETURN_FALSE_IF_NULL(audio_stream_, "avformat_new_stream");

  audio_stream_->id = AUDIO_STREAM_INDEX;

  audio_cc_ = avcodec_alloc_context3(audio_codec_);
  RETURN_FALSE_IF_NULL(audio_cc_, "avcodec_alloc_context3");

  // audio_cc_->codec_type = AVMEDIA_TYPE_AUDIO;
  // audio_cc_->bit_rate_tolerance = 6400 * 1000 * 3 / 2;
  // audio_cc_->frame_size = 1024;

  audio_cc_->codec_id = codec_id;
  audio_cc_->time_base = (AVRational){ 1, audio_sample_rate_ };
  audio_cc_->sample_rate = audio_sample_rate_;
  audio_cc_->channel_layout = audio_channel_layout_;
  audio_cc_->channels = av_get_channel_layout_nb_channels(audio_cc_->channel_layout);
  audio_cc_->sample_fmt = audio_sample_fmt_;
  //audio_cc_->bit_rate = audio_bit_rate_;
  audio_stream_->time_base = audio_cc_->time_base; //(AVRational){ 1, audio_cc_->sample_rate };

  assert(fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER);
  audio_cc_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  // audio_cc_->extradata_size = 2;
  // audio_cc_->extradata = (uint8_t *)av_mallocz(2);
  // audio_cc_->extradata[0] = 0;
  // audio_cc_->extradata[1] = 0;

  // av_dict_set(&fmt_ctx_->metadata, "hasAudio", "true", 0);

  return true;
}

bool Out::open_audio() {
  int iret = -1;
  AVDictionary *opt = NULL;

  av_dict_copy(&opt, opt_, 0);
  iret = avcodec_open2(audio_cc_, audio_codec_, NULL);
  av_dict_free(&opt);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("avcodec_open2", iret);
    return false;
  }

  AVKID_LOG_DEBUG << "audio frame size:" << audio_cc_->frame_size << "\n";
  int nb_samples = audio_cc_->frame_size;

  iret = avcodec_parameters_from_context(audio_stream_->codecpar, audio_cc_);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("avcodec_parameters_from_context", iret);
    return false;
  }

  return true;
}

bool Out::add_video_stream() {
  AVCodecID codec_id = AV_CODEC_ID_H264;
  // AVCodecID codec_id = AV_CODEC_ID_FLV1;

  video_codec_ = avcodec_find_encoder(codec_id);
  if (!video_codec_) {
    AVKID_LOG_INFO << "Func avcodec_find_encoder failed.\n";
    return false;
  }

  video_stream_ = avformat_new_stream(fmt_ctx_, NULL);
  if (!video_stream_) {
    AVKID_LOG_INFO << "Func avformat_new_stream failed.\n";
    return false;
  }

  video_stream_->id = VIDEO_STREAM_INDEX;

  video_cc_ = avcodec_alloc_context3(video_codec_);
  if (!video_cc_) {
    AVKID_LOG_INFO << "Func avcodec_alloc_context3 failed.\n";
    return false;
  }

  video_cc_->bit_rate = bit_rate_;
  // video_cc_->bit_rate = 400000;
  video_cc_->width = width_;
  // video_cc_->width = 352;
  video_cc_->height = height_;
  // video_cc_->height = 288;
  video_cc_->gop_size = gop_size_;
  // video_cc_->gop_size = 12;
  video_cc_->pix_fmt = pix_fmt_;
  // video_cc_->pix_fmt = AV_PIX_FMT_YUV420P;
  // video_cc_->frame_size = 0;
  video_cc_->framerate.num = 15;
  video_cc_->framerate.den = 1;

  video_cc_->codec_id = codec_id;
  video_stream_->time_base = (AVRational){ 1, video_frame_rate_};
  video_cc_->time_base = video_stream_->time_base;

  // extradata
  if (fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER) {
    video_cc_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  if (1) {
    uint8_t extradata[1024] = {0};
    int extradata_size = 0;
    serialize_to_extradata(sps_len_, sps_data_, pps_len_, pps_data_, extradata, &extradata_size);
    video_cc_->extradata = (uint8_t *)av_mallocz(extradata_size);
    memcpy(video_cc_->extradata, extradata, extradata_size);
    video_cc_->extradata_size = extradata_size;
    //video_stream_->codecpar->extradata = video_cc_->extradata;
    //video_stream_->codecpar->extradata_size = video_cc_->extradata_size;
  }

  //metadata
  //AVRational display_aspect_ratio;
  //av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
  //          width_, height_, 1024*1024);

  //char buf[128];
  //snprintf(buf, 127, "%d:%d", display_aspect_ratio.num, display_aspect_ratio.den);

  //av_dict_set(&fmt_ctx_->metadata, "framerate", "15", AV_DICT_DONT_OVERWRITE);
  //av_dict_set(&fmt_ctx_->metadata, "display_aspect_ratio", buf, 0);
  //av_dict_set(&fmt_ctx_->metadata, "hasVideo", "true", 0);

  return true;
}

bool Out::open_video() {
  int iret = -1;
  AVDictionary *opt = NULL;

  av_dict_copy(&opt, opt_, 0);
  iret = avcodec_open2(video_cc_, video_codec_, &opt);
  av_dict_free(&opt);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("avcodec_open2", iret);
    return false;
  }

  iret = avcodec_parameters_from_context(video_stream_->codecpar, video_cc_);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("avcodec_parameters_from_context", iret);
    return false;
  }

  return true;
}

bool Out::open(const std::string &url, uint64_t timeout_ms) {
  assert(has_audio_ || has_video_);

  AVKID_LOG_DEBUG << "> Out::open url:" << url << " timeout_ms:" << timeout_ms << "\n";

  int iret = -1;
  bool bret = false;

  avformat_alloc_output_context2(&fmt_ctx_, NULL, "flv", url.c_str());
  if (!fmt_ctx_) {
    AVKID_LOG_INFO << "Func avformat_alloc_output_context2 failed.\n";
    return false;
  }

  url_ = url;
  open_ms_ = chef::stuff_op::tick_msec();
  timeout_ms_ = timeout_ms;
  fmt_ctx_->interrupt_callback.callback = Out::interrupt_cb;
  fmt_ctx_->interrupt_callback.opaque = (void *)this;

  if (has_video_) {
    fmt_ctx_->oformat->video_codec = AV_CODEC_ID_H264;
    bret = add_video_stream();
    assert(bret);
  }
  if (has_audio_) {
    fmt_ctx_->oformat->audio_codec = AV_CODEC_ID_AAC;
    bret = add_audio_stream();
    assert(bret);
  }

  if (has_video_) {
    bret = open_video();
    assert(bret);
  }
  if (has_audio_) {
    bret = open_audio();
    assert(bret);
  }

  av_dump_format(fmt_ctx_, 0, url.c_str(), 1);

  if (!(fmt_ctx_->flags & AVFMT_NOFILE)) {
    iret = avio_open(&fmt_ctx_->pb, url.c_str(), AVIO_FLAG_WRITE);
    if (iret < 0) {
      FFMPEG_FAILED_LOG("avio_open", iret);
      return false;
    }
  }

  iret = avformat_write_header(fmt_ctx_, &opt_);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("avformat_write_header", iret);
    return false;
  }

  // if (!(fmt_ctx_->flags & AVFMT_NOFILE)) {
  //   avio_closep(&fmt_ctx_->pb);
  // }

  return true;
}

void Out::write_packet(AVPacket *av_packet, bool is_audio) {
  is_audio ? write_audio_packet(av_packet) : write_video_packet(av_packet);
  //av_packet_unref(av_packet);
}

void Out::write_audio_packet(AVPacket *packet) {
  int iret = -1;

  //AVKID_LOG_DEBUG << "write audio frame origin. -b pts:" << packet->pts
  //                     << " nb_samples:" << audio_cc_->frame_size
  //                     << "\n";
  packet->pts = av_rescale_q(audio_samples_count, (AVRational){1, audio_cc_->sample_rate}, audio_cc_->time_base);
  //AVKID_LOG_DEBUG << "write audio frame origin. -a pts:" << packet->pts
  //                     << " nb_samples:" << audio_cc_->frame_size
  //                     << "\n";
  audio_samples_count += audio_cc_->frame_size;

  av_packet_rescale_ts(packet, audio_cc_->time_base, audio_stream_->time_base);
  packet->dts = packet->pts;
  //AVKID_LOG_DEBUG << "CHEF -a audio pts:" << packet.pts << " dts:" << packet.dts << "\n";
  packet->stream_index = audio_stream_->index;

  //__log_packet(fmt_ctx_, packet);

  iret = av_interleaved_write_frame(fmt_ctx_, packet);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("av_interleaved_write_frame", iret);
    return;
  }
}

void Out::write_video_packet(AVPacket *packet) {
  uint8_t nal_unit_type = packet->data[4] & 0x1f;
  if (nal_unit_type == CHEF_H264_NAL_UNIT_TYPE_IDR_SLICE) {
    uint8_t extradata[1024] = {0};
    int extradata_size = 0;
    AVKID_LOG_DEBUG << "CHEFERASEME sps_len:" << sps_len_ << " pps_len:" << pps_len_ << "\n";
    serialize_to_extradata(sps_len_, sps_data_, pps_len_, pps_data_, extradata, &extradata_size);

    uint8_t* psd = av_packet_new_side_data(packet, AV_PKT_DATA_NEW_EXTRADATA, extradata_size);
    memcpy(psd, extradata, extradata_size);
  }

  packet->pts = video_next_pts_++;
  av_packet_rescale_ts(packet, video_cc_->time_base, video_stream_->time_base);
  packet->dts = packet->pts;
  packet->stream_index = video_stream_->index;

  //__log_packet(fmt_ctx_, packet);

  int iret = av_interleaved_write_frame(fmt_ctx_, packet);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("av_interleaved_write_frame", iret);
    return;
  }
}

void Out::write_frame(AVFrame *av_frame, bool is_audio) {
  is_audio ? write_audio_frame(av_frame) : write_video_frame(av_frame);
}

void Out::write_audio_frame(AVFrame *av_frame) {
  if (!has_audio_) { return; }

  int iret = 0;
  AVPacket packet = {0};

  //AVKID_LOG_DEBUG << "write audio frame origin. -b pts:" << av_frame->pts
  //                     << " nb_samples:" << av_frame->nb_samples
  //                     << "\n";
  av_frame->pts = av_rescale_q(audio_samples_count, (AVRational){1, audio_cc_->sample_rate}, audio_cc_->time_base);
  //AVKID_LOG_DEBUG << "write audio frame origin. -a pts:" << av_frame->pts
  //                     << " nb_samples:" << av_frame->nb_samples
  //                     << "\n";
  audio_samples_count += av_frame->nb_samples;

  iret = avcodec_send_frame(audio_cc_, av_frame);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("avcodec_send_frame", iret);
    return;
  }
  while (iret >= 0) {
    iret = avcodec_receive_packet(audio_cc_, &packet);
    if (iret < 0) {
      if (iret != AVERROR(EAGAIN)) {
        FFMPEG_FAILED_LOG("avcodec_receive_frame", iret);
      }
      return;
    }

    AVKID_LOG_DEBUG << "CHEF -b audio pts:" << packet.pts << " dts:" << packet.dts <<
                         " tb:" << audio_cc_->time_base.den << " " << audio_cc_->time_base.num <<
                         " tb2:" << audio_stream_->time_base.den << " " << audio_stream_->time_base.num << "\n";

    av_packet_rescale_ts(&packet, audio_cc_->time_base, audio_stream_->time_base);
    //av_packet_rescale_ts(&packet, audio_stream_->time_base, audio_cc_->time_base);
    //AVKID_LOG_DEBUG << "CHEF -a audio pts:" << packet.pts << " dts:" << packet.dts << "\n";
    packet.stream_index = audio_stream_->index;

    //__log_packet(fmt_ctx_, &packet);

    iret = av_interleaved_write_frame(fmt_ctx_, &packet);
    if (iret < 0) {
      FFMPEG_FAILED_LOG("av_interleaved_write_frame", iret);
      return;
    }
  }

}

void Out::write_video_frame(AVFrame *av_frame) {
  if (!has_video_) { return; }

  int iret = 0;
  AVPacket packet = {0};

  av_frame->pts = video_next_pts_++;

  iret = avcodec_send_frame(video_cc_, av_frame);
  if (iret < 0) {
    FFMPEG_FAILED_LOG("avcodec_send_frame", iret);
    return;
  }
  while (iret >= 0) {
    iret = avcodec_receive_packet(video_cc_, &packet);
    if (iret < 0) {
      if (iret != AVERROR(EAGAIN)) {
        FFMPEG_FAILED_LOG("avcodec_receive_frame", iret);
      }
      return;
    }

    av_packet_rescale_ts(&packet, video_cc_->time_base, video_stream_->time_base);
    packet.stream_index = video_stream_->index;

    //__log_packet(fmt_ctx_, &packet);

    iret = av_interleaved_write_frame(fmt_ctx_, &packet);
    if (iret < 0) {
      FFMPEG_FAILED_LOG("av_interleaved_write_frame", iret);
      return;
    }
  }

  av_frame_unref(av_frame);

  //if (av_compare_ts(video_next_pts_, video_cc_->time_base, 10.0, (AVRational){1, 1}) >= 0) {
  //  if (!(fmt_ctx_->flags & AVFMT_NOFILE)) {
  //    avio_closep(&fmt_ctx_->pb);
  //  }
  //  exit(0);
  //}
}

void Out::packet_cb(AVPacket *av_packet, bool is_audio) {
  if (input_type_ == INPUT_TYPE_AV_FRAME) { return; }
  if (is_audio && !has_audio_) { return; }
  if (!is_audio && !has_video_) { return; }
  //AVKID_LOG_DEBUG << "CHEFERASEME origin video packet pts and dts:" << av_packet->pts << " " << av_packet->dts << "\n";
  //async_do_packet(av_packet, is_audio);
  AVPacket *packet = av_packet_clone(av_packet);
  write_packet(packet, is_audio);
  //av_packet_unref(packet);
}

void Out::frame_cb(AVFrame *av_frame, bool is_audio) {
  //AVKID_LOG_DEBUG << "CHEFERASEME origin video frame pts and dts:" << av_frame->pts << " " << av_frame->pkt_dts << "\n";
  async_do_frame(av_frame, is_audio);
}

void Out::async_do_packet(AVPacket *av_packet, bool is_audio) {
  if (input_type_ == INPUT_TYPE_AV_FRAME) { return; }
  if (is_audio && !has_audio_) { return; }
  if (!is_audio && !has_video_) { return; }

  AVPacket *packet = av_packet_clone(av_packet);
  thread_->add(chef::bind(&Out::write_packet, this, packet, is_audio));
}

void Out::async_do_frame(AVFrame *av_frame, bool is_audio) {
  if (input_type_ == INPUT_TYPE_AV_PACKET) { return; }
  if (is_audio && !has_audio_) { return; }
  if (!is_audio && !has_video_) { return; }

  AVFrame *frame = av_frame_clone(av_frame);
  thread_->add(chef::bind(&Out::write_frame, this, frame, is_audio));
}

}; // namespace avkid
