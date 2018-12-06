/**
 * @file   avkid_help_op.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

#define AVKID_H264_NAL_UNIT_TYPE_MASK      0x1F
#define AVKID_H264_NAL_UNIT_TYPE_SLICE     0x01
#define AVKID_H264_NAL_UNIT_TYPE_IDR_SLICE 0x05
#define AVKID_H264_NAL_UNIT_TYPE_SEI       0x06
#define AVKID_H264_NAL_UNIT_TYPE_SPS       0x07
#define AVKID_H264_NAL_UNIT_TYPE_PPS       0x08
#define AVKID_H264_NAL_UNIT_TYPE_AUD       0x09

class HelpOP {
  public:
    static void global_init_ffmpeg();
    static void global_deinit_ffmpeg();
    static std::string av_make_error_string(int errnum);
    static std::string stringify_ffmpeg_error(int err);

    static void deserialize_from_extradata(uint8_t *extradata,
                                           int extradata_size,
                                           unsigned short *sps_len /*out*/,
                                           uint8_t **sps_data /*out*/,
                                           unsigned short *pps_len /*out*/,
                                           uint8_t **pps_data /*out*/);

    // 传出参数 extradata extradata_size @NOTICE 空间由外部申请
    static void serialize_to_extradata(unsigned short sps_len,
                                       uint8_t *sps_data,
                                       unsigned short pps_len,
                                       uint8_t *pps_data,
                                       uint8_t *extradata /*out*/,
                                       int *extradata_size /*out*/);

    static AVFrame *scale_video_frame(AVFrame *frame, int width, int height);
    static bool dump_mjpeg(AVFrame *frame, const std::string &filename);

    static bool mix_video_pin_frame(AVFrame *bg, AVFrame *part, int x, int y);

    static int open_fmt_ctx_with_timtout(AVFormatContext **fmt_ctx, const std::string &url, uint32_t timeout_msec);
    static int open_codec_context(int *stream_idx /*out*/,
                                  AVCodecContext **codec_ctx /*out*/,
                                  AVFormatContext *fmt_ctx,
                                  enum AVMediaType type,
                                  bool is_decode,
                                  int width=-1,
                                  int height=-1);

    static AVFrame *share_frame(AVFrame *frame);
    static AVPacket *share_packet(AVPacket *packet);
    static void unshare_frame(AVFrame *frame);
    static void unshare_packet(AVPacket *packet);
};

}
