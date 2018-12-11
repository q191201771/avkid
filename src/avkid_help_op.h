/**
 * @file   avkid_help_op.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class HelpOP {
  public:
    static void global_init_ffmpeg();
    static void global_deinit_ffmpeg();
    static std::string av_make_error_string(int errnum);
    static std::string stringify_ffmpeg_error(int err);

  public:
    static int open_fmt_ctx_with_timtout(AVFormatContext **fmt_ctx, const std::string &url, uint32_t timeout_msec);
    static int open_codec_context(int *stream_idx /*out*/,
                                  AVCodecContext **codec_ctx /*out*/,
                                  AVFormatContext *fmt_ctx,
                                  enum AVMediaType type,
                                  bool is_decode,
                                  int width=-1,
                                  int height=-1);

  public:
    static bool dump_mjpeg(AVFrame *frame, const std::string &filename);

  public:
    // 对图形进行缩放
    static AVFrame *scale_video_frame(AVFrame *src, int width, int height);

    // 截取<frame>图形指定位置、指定大小部分
    // 简单来说，得到的结果是原始图形的一部分
    // 注意，如果width和height指定为-1，则截取至原始图形的右下角
    static AVFrame *cut_video_frame(AVFrame *frame, int x, int y, int width=-1, int height=-1);

    // 将<part>绘制到<bg>上，<part>的宽高应小于<bg>的宽高
    // <x>和<y>表示绘制到<bg>上时，从<bg>的哪个坐标开始
    // 简单来说，是将<part>的整体作为<bg>的一部分，贴到<bg>上
    static bool mix_video_pin_frame(AVFrame *bg, AVFrame *part, int bg_x, int bg_y);

  public:
    static AVFrame *frame_alloc_prop();
    static void frame_alloc_buf(AVFrame *frame, bool is_audio);
    static AVFrame *frame_alloc_copy_prop_ref_buf(AVFrame *frame);
    static void frame_free_prop_unref_buf(AVFrame **frame);
    static void frame_unref_buf(AVFrame *frame);

    static AVPacket *packet_alloc_prop_ref_buf(AVPacket *packet);
    static void packet_free_prop_unref_buf(AVPacket **packet);
    static void packet_unref_buf(AVPacket *packet);

  public:
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

};

}
