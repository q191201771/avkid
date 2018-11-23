#include "avkid_in.h"
#include <assert.h>
#include "chef_stuff_op.hpp"
#include "avkid_log_adapter.hpp"

namespace avkid {

static int __open_codec_context(int *stream_idx /*out*/,
                              AVCodecContext **dec_ctx /*out*/,
                              AVFormatContext *fmt_ctx,
                              enum AVMediaType type,
                              bool refcount=false)
{
  int ret, stream_index;
  AVStream *st;
  AVCodec *dec = NULL;
  AVDictionary *opts = NULL;

  if ((ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0)) < 0) {
    FFMPEG_FAILED_LOG("av_find_best_stream", ret);
    return ret;
  }

  stream_index = ret;
  st = fmt_ctx->streams[stream_index];

  // 28 AV_CODEC_ID_H264
  // 86018 AV_CODEC_ID_AAC
  dec = avcodec_find_decoder(st->codecpar->codec_id);
  if (!dec) {
    return AVERROR(EINVAL);
  }

  *dec_ctx = avcodec_alloc_context3(dec);
  if (!*dec_ctx) {
    return AVERROR(ENOMEM);
  }

  if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
    FFMPEG_FAILED_LOG("avcodec_parameters_to_context", ret);
    return ret;
  }

  // av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
  if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
    FFMPEG_FAILED_LOG("avcodec_open2", ret);
    return ret;
  }

  *stream_idx = stream_index;

  return 0;
}


In::~In() {
  if (audio_dec_ctx_) {
    avcodec_free_context(&audio_dec_ctx_);
    audio_dec_ctx_ = NULL;
  }
  if (video_dec_ctx_) {
    avcodec_free_context(&video_dec_ctx_);
    video_dec_ctx_ = NULL;
  }
  if (fmt_ctx_) {
    avformat_close_input(&fmt_ctx_);
    fmt_ctx_ = NULL;
  }

}

// @NOTICE return 0 continue 1 exit
int In::interrupt_cb(void *opaque) {
  In *obj = (In *)opaque;
  if (obj->opened_ == false && obj->open_ms_ != 0 && obj->timeout_ms_ != 0 &&
      (chef::stuff_op::tick_msec() - obj->open_ms_ > obj->timeout_ms_)
  ) {
    AVKID_LOG_INFO << "Timeout. interrupt_cb." << "\n";
    return 1;
  }
  return 0;
}

bool In::open(const std::string &url, uint64_t timeout_ms) {
  AVKID_LOG_DEBUG << "> In::open url:" << url << " timeout_ms:" << timeout_ms << "\n";

  int iret = -1;

  fmt_ctx_ = avformat_alloc_context();
  RETURN_FALSE_IF_NULL(fmt_ctx_, "avformat_alloc_context");

  url_ = url;
  open_ms_ = chef::stuff_op::tick_msec();
  timeout_ms_ = timeout_ms;
  fmt_ctx_->interrupt_callback.callback = In::interrupt_cb;
  fmt_ctx_->interrupt_callback.opaque = (void *)this;

  iret = avformat_open_input(&fmt_ctx_, url.c_str(), NULL, NULL);
  opened_ = true;
  // @NOTICE 错误值举例
  // -875574520, AVERROR_HTTP_NOT_FOUND, Server returned 404 Not Found, rtmp流不存在
  // -2, , No such file or directory, 文件不存在
  // -1414092869, , Immediate exit requested, 被interrupt_cb干掉了
  RETURN_FALSE_IF_FFMPEG_FAILED("avformat_open_input", iret);

  iret = avformat_find_stream_info(fmt_ctx_, NULL);
  RETURN_FALSE_IF_FFMPEG_FAILED("avformat_find_stream_info", iret);

  iret = __open_codec_context((int *)&video_stream_index_, &video_dec_ctx_, fmt_ctx_, AVMEDIA_TYPE_VIDEO);
  RETURN_FALSE_IF_FFMPEG_FAILED("__open_codec_context", iret);
  AVStream *video_stream = fmt_ctx_->streams[video_stream_index_];

  iret = __open_codec_context((int *)&audio_stream_index_, &audio_dec_ctx_, fmt_ctx_, AVMEDIA_TYPE_AUDIO);
  RETURN_FALSE_IF_FFMPEG_FAILED("__open_codec_context", iret);
  AVStream *audio_stream = fmt_ctx_->streams[audio_stream_index_];


  AVKID_LOG_DEBUG << "dec video width:" << video_dec_ctx_->width
                       << ", height:" << video_dec_ctx_->height
                       << ", pix_fmt:" << video_dec_ctx_->pix_fmt
                       << ", gop_size:" << video_dec_ctx_->gop_size
                       << ", bit_rate:" << video_dec_ctx_->bit_rate
                       << ", time_base:" << video_dec_ctx_->time_base.den << " " << video_dec_ctx_->time_base.num << "\n";
  AVKID_LOG_DEBUG << "dec audio channels:" << audio_dec_ctx_->channels
                       << ", sample_fmt:" << audio_dec_ctx_->sample_fmt
                       << ", bit_rate:" << audio_dec_ctx_->bit_rate << "\n";
  AVKID_LOG_DEBUG << "video time_base:" << video_stream->time_base.num << "/" << video_stream->time_base.den << "\n";
  AVKID_LOG_DEBUG << "video frame_rate:" << video_stream->r_frame_rate.num << "/" << video_stream->r_frame_rate.den << "\n";
  // AVKID_LOG_DEBUG << "audio time_base:" << audio_stream->time_base.num << "/" << audio_stream->time_base.den << "\n";

  video_frame_rate_ = int((float)video_stream->r_frame_rate.num / video_stream->r_frame_rate.den);

  // @NOTICE obtain sps pps
  assert(video_stream->codecpar->extradata_size > 0);
  deserialize_from_extradata(video_stream->codecpar->extradata,
                             video_stream->codecpar->extradata_size,
                             &sps_len_,
                             &sps_data_,
                             &pps_len_,
                             &pps_data_);

  AVKID_LOG_INFO << "-----video info-----\n";
  AVKID_LOG_INFO << "extradata_size:" << video_stream->codecpar->extradata_size << "\n";
  AVKID_LOG_INFO << "extradata:" << chef::stuff_op::bytes_to_hex(video_stream->codecpar->extradata, video_stream->codecpar->extradata_size).c_str() << "\n";
  AVKID_LOG_INFO << "sps_len:" << sps_len_ << "\n";
  AVKID_LOG_INFO << "sps_data:" << chef::stuff_op::bytes_to_hex(sps_data_, sps_len_).c_str() << "\n";
  AVKID_LOG_INFO << "pps_len:" << pps_len_ << "\n";
  AVKID_LOG_INFO << "pps_data:" << chef::stuff_op::bytes_to_hex(pps_data_, pps_len_).c_str() << "\n";
  // AVKID_LOG_INFO << "-----audio info-----\n";
  // AVKID_LOG_INFO << "--------------------\n";

  av_dump_format(fmt_ctx_, 0, url.c_str(), 0);

  return true;
}

bool In::read() {
  int iret = -1;

  AVFrame *frame = av_frame_alloc();
  RETURN_FALSE_IF_NULL(frame, "av_frame_alloc");

  AVPacket pkt = {0};

  while(!stop_flag_ && av_read_frame(fmt_ctx_, &pkt) >= 0) {
    if (observer_) { observer_->packet_cb(&pkt, pkt.stream_index == audio_stream_index_); }

    if (!should_decode_) {
      av_packet_unref(&pkt);
      continue;
    }

    // 分析AVPacket
    if (pkt.stream_index == audio_stream_index_) {
      // AVKID_LOG_DEBUG << "Audio packet. " << pkt.pts << " " << pkt.dts << " " << pkt.duration << " "
      //                     << pkt.pos << " " << pkt.size << "\n";
    } else if (pkt.stream_index == video_stream_index_) {
      if ((pkt.flags & AV_PKT_FLAG_KEY) != 0) {
        // @NOTICE IFrame
      }
      // AVKID_LOG_INFO << "Video packet. " << pkt.flags << " " << pkt.pts << " " << pkt.dts << " "
      //                     << pkt.duration << " " << pkt.pos << " " << pkt.size << "\n";

      // @NOTICE head 4 bytes means size, may more than one packet
      uint32_t pos = 0;
      int i = 0;
      for (; ; i++) {
        uint32_t calc_size = ntohl(*((uint32_t *)(pkt.data + pos)));
        uint8_t nal_unit_type = *(pkt.data + pos + 4) & CHEF_H264_NAL_UNIT_TYPE_MASK;
        if (nal_unit_type != CHEF_H264_NAL_UNIT_TYPE_SLICE &&
            nal_unit_type != CHEF_H264_NAL_UNIT_TYPE_IDR_SLICE &&
            nal_unit_type != CHEF_H264_NAL_UNIT_TYPE_SEI &&
            nal_unit_type != CHEF_H264_NAL_UNIT_TYPE_SPS &&
            nal_unit_type != CHEF_H264_NAL_UNIT_TYPE_PPS
        ) {
          AVKID_LOG_WARN << "Unknown nal type:" << nal_unit_type << "\n";
        }
        //AVKID_LOG_INFO << "Nal unit type:" << (int)nal_unit_type
        //               << ", i:" << i << ", pos:" << pos << ", calc_size:" << calc_size << ", pk.size:" << pkt.size
        //               << "\n";

        // if (false && nal_header_type == CHEF_NAL_HEADER_TYPE_SEI) {
        //   AVKID_LOG_INFO << "calc size:" << calc_size << "\n";
        //   AVKID_LOG_INFO << (pkt.data + pos + 8) << "\n";
        //   AVKID_LOG_INFO << "SEI:\n" << chef::stuff_op::bytes_to_hex(pkt.data + pos, calc_size).c_str() << "\n";
        // }
        // if (true && nal_header_type == CHEF_NAL_HEADER_TYPE_SPS) {
        //   AVKID_LOG_INFO << "SPS:\n" << chef::stuff_op::bytes_to_hex(pkt.data + pos, calc_size).c_str() << "\n";
        // }
        // if (true && nal_header_type == CHEF_NAL_HEADER_TYPE_B_SLICE) {
        //   AVKID_LOG_INFO << "B frame.\n";
        // }

        // AVKID_LOG_INFO << i << ": " << " pos:" << pos << " nal_header_type:" << int(nal_header_type) << "\n";
        // @NOTICE 一个AVPacket钟可能有多个NAL
        pos += 4 + calc_size;
        if (pos >= pkt.size) {
          break;
        }
        //assert(pos < pkt.size);
      }
      if (i != 0) {
        // AVKID_LOG_INFO << "CHEFNOTICEME multi\n";
      }
    }
    // printf("%s", chef::stuff_op::bytes_to_hex(pkt.data, pkt.size > 8 ? 8 : pkt.size).c_str());
    // printf(">>>>>\n");

    // AVPacket -> AVFrame
    uint8_t nal_unit_type = *(pkt.data + 4) & CHEF_H264_NAL_UNIT_TYPE_MASK;
    if (nal_unit_type == CHEF_H264_NAL_UNIT_TYPE_SEI) {
      av_packet_unref(&pkt);
      continue;
    }
    iret = avcodec_send_packet(pkt.stream_index == audio_stream_index_ ? audio_dec_ctx_ : video_dec_ctx_, &pkt);
    if (iret < 0) {
      FFMPEG_FAILED_LOG("avcodec_send_packet", iret);
      av_packet_unref(&pkt);
      continue;
    }
    while (iret >= 0) {
      iret = avcodec_receive_frame(pkt.stream_index == audio_stream_index_ ? audio_dec_ctx_ : video_dec_ctx_, frame);
      if (iret < 0) {
        if (iret != AVERROR(EAGAIN)) {
          FFMPEG_FAILED_LOG("avcodec_receive_frame", iret);
        }
        break;
      }

      //AVKID_LOG_DEBUG << "CHEFERASEME " << frame->pkt_size << " " << frame->linesize[0] << " " << frame->linesize[1] << " " << frame->linesize[2]
      //  << " " << frame->linesize[3] << " " << frame->linesize[4] << " " << frame->linesize[5]
      //  << " " << frame->linesize[6] << " " << frame->linesize[7]
      //  << " "<< (pkt.stream_index == audio_stream_index_ ? "audio" : "video") << "\n";

      if (observer_) {
        observer_->frame_cb(frame, pkt.stream_index == audio_stream_index_);
      }
    }

    av_packet_unref(&pkt);
    av_frame_unref(frame);
  }

  return true;
}

}; // namespace avkid
