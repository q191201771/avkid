/**
 * @file   avkid_common.hpp
 * @author chef
 *
 */

#pragma once

#include <arpa/inet.h>
#include <assert.h>
#include <cinttypes>
#include <sstream>
#include "avkid_log_adapter.hpp"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

extern "C" {
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/samplefmt.h>
  #include <libavutil/timestamp.h>
  #include <libavfilter/avfilter.h>
  #include <libswscale/swscale.h>
}

namespace avkid {

#define CHEF_H264_NAL_UNIT_TYPE_MASK      0x1F
#define CHEF_H264_NAL_UNIT_TYPE_SLICE     1
#define CHEF_H264_NAL_UNIT_TYPE_IDR_SLICE 5
#define CHEF_H264_NAL_UNIT_TYPE_SEI       6
#define CHEF_H264_NAL_UNIT_TYPE_SPS       7
#define CHEF_H264_NAL_UNIT_TYPE_PPS       8
//H264_NAL_DPA             = 2,
//H264_NAL_DPB             = 3,
//H264_NAL_DPC             = 4,
//H264_NAL_AUD             = 9,
//H264_NAL_END_SEQUENCE    = 10,
//H264_NAL_END_STREAM      = 11,
//H264_NAL_FILLER_DATA     = 12,
//H264_NAL_SPS_EXT         = 13,
//H264_NAL_AUXILIARY_SLICE = 19,

// 一般来说，整个字节称为NAL header，低5位称为NAL unit type
//#define CHEF_NAL_HEADER_TYPE_B_SLICE 0x01 // 1
//#define CHEF_NAL_HEADER_TYPE_SEI     0x06 // 6
//#define CHEF_NAL_HEADER_TYPE_P_SLICE 0x41 // 65
//#define CHEF_NAL_HEADER_TYPE_IDR     0x65 // 101
//#define CHEF_NAL_HEADER_TYPE_SPS     0x67 // 103
//#define CHEF_NAL_HEADER_TYPE_PPS     0x68 // 104

// #define CHEF_SPS_PROFILE_IDC_PROFILE_BASELINE 0x66
// #define CHEF_SPS_PROFILE_IDC_PROFILE_MAIN     0x77
// #define CHEF_SPS_PROFILE_IDC_PROFILE_EXTENDED 0x88

static void global_init_ffmpeg() {
  av_log_set_level(AV_LOG_DEBUG);

  // for lavf
  av_register_all();
  // for lavf
  avformat_network_init();
  // avfilter_register_all();
}

static void global_deinit_ffmpeg() {
  avformat_network_deinit();
}

static std::string stringify_ffmpeg_error(int err) {
  std::ostringstream ss;
  ss << "(" << err << ":" << av_err2str(err) << ")";
  return ss.str();
}

// 传出参数 sps_len sps_data pps_len pps_data
static void deserialize_from_extradata(uint8_t *extradata,
                                       int extradata_size,
                                       unsigned short *sps_len,
                                       uint8_t **sps_data,
                                       unsigned short *pps_len,
                                       uint8_t **pps_data)
{
    *sps_len  = ntohs(*(unsigned short *)(extradata + 6));
    *sps_data = extradata + 6 + sizeof(*sps_len);
    *pps_len  = ntohs(*(unsigned short *)(extradata + 6 + sizeof(*sps_len) + *sps_len + 1));
    *pps_data = extradata + 6 + sizeof(*sps_len) + *sps_len + 1 + sizeof(*pps_len);
}

// 传出参数 extradata extradata_size @NOTICE 空间由外部申请
static void serialize_to_extradata(unsigned short sps_len,
                                   uint8_t *sps_data,
                                   unsigned short pps_len,
                                   uint8_t *pps_data,
                                   uint8_t *extradata,
                                   int *extradata_size)
{
  uint8_t *p = extradata;
  p[0] = 0x01;
  p[1] = sps_data[1];
  p[2] = sps_data[2];
  p[3] = sps_data[3];
  p[4] = 0xff;
  p[5] = 0xe1;

  int i = 6;
  unsigned short sps_len_ns = htons(sps_len);
  memcpy(&p[i], (const void *)(&sps_len_ns), 2);
  i += 2;
  memcpy(&p[i], sps_data, sps_len);
  i += sps_len;

  p[i++] = 0x01;
  unsigned short pps_len_ns = htons(pps_len);
  memcpy(&p[i], (const void *)(&pps_len_ns), 2);
  i += 2;
  memcpy(&p[i], pps_data, pps_len);
  i += pps_len;

  *extradata_size = i;
}

static bool dump_mjpeg(AVFrame *frame, const std::string &filename) {
  int ret;
  bool bret = false;

  AVFormatContext *fmt_ctx = NULL;
  AVCodec *codec = NULL;
  AVStream *stream = NULL;
  AVCodecContext *cc = NULL;
  AVPacket packet;
  AVCodecID codec_id = AV_CODEC_ID_MJPEG;

  avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, filename.c_str());
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

  stream = avformat_new_stream(fmt_ctx, NULL);
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

  ret = avcodec_open2(cc, codec, NULL);
  if (ret < 0) {
    FFMPEG_FAILED_LOG("avcodec_open2", ret);
    goto END;
  }

  ret = avcodec_parameters_from_context(stream->codecpar, cc);
  if (ret < 0) {
    FFMPEG_FAILED_LOG("avcodec_parameters_from_context", ret);
    goto END;
  }

  ret = avformat_write_header(fmt_ctx, NULL);
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

static AVFrame *__alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples) {
  AVFrame *frame = av_frame_alloc();
  int ret;

  if (!frame) {
    AVKID_LOG_INFO << "Func alloc_audio_frame failed.\n";
    return NULL;
  }

  frame->format = sample_fmt;
  frame->channel_layout = channel_layout;
  frame->sample_rate = sample_rate;
  frame->nb_samples = nb_samples;

  if (nb_samples) {
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
      FFMPEG_FAILED_LOG("av_frame_get_buffer", ret);
      return NULL;
    }
  }

  return frame;
}

static AVFrame *__alloc_video_frame(enum AVPixelFormat pix_fmt, int width, int height) {
  AVFrame *picture;
  int ret;

  picture = av_frame_alloc();
  if (!picture) {
    return NULL;
  }

  picture->format = pix_fmt;
  picture->width = width;
  picture->height = height;

  ret = av_frame_get_buffer(picture, 32);
  if (ret < 0) {
    FFMPEG_FAILED_LOG("av_frame_get_buffer", ret);
    return NULL;
  }
  return picture;
}

}; // namespace avkid
