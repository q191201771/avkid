/**
 * @file   avkid_out.h
 * @author chef
 *
 */

#pragma once

#include <string>
#include "avkid_common.hpp"
#include "avkid_in.h"
#include "chef_task_thread.hpp"

// @NOTICE 只支持h264和AAC

namespace avkid {

class Out : public InObserver {
  public:
    Out();
    ~Out();

    void init_video(AVPixelFormat pix_fmt,
                    int width,
                    int height,
                    int gop_size,
                    int64_t bit_rate,
                    int video_frame_rate,
                    unsigned short sps_len,
                    uint8_t *sps_data,
                    unsigned short pps_len,
                    uint8_t *pps_data);

    void init_audio(AVSampleFormat audio_sample_fmt, int audio_bit_rate, int audio_sample_rate,
                    int audio_channel_layout);

    bool open(const std::string &url, uint64_t timeout_ms=0);

    void async_encode_frame(AVFrame *av_frame, bool is_audio);

  public:
    void write_packet(AVPacket *av_packet, bool is_audio);
    void write_video_packet(AVPacket *av_packet);
    void write_audio_packet(AVPacket *av_packet);
    void write_frame(AVFrame *av_frame, bool is_audio);
    void write_video_frame(AVFrame *av_frame);
    void write_audio_frame(AVFrame *av_frame);

  public:
    virtual void packet_cb(AVPacket *av_packet, bool is_audio);
    virtual void frame_cb(AVFrame *av_frame, bool is_audio);

  private:
    bool add_audio_stream();
    bool add_video_stream();
    bool open_audio();
    bool open_video();

  private:
    static int interrupt_cb(void *opaque);

  private:
    std::string url_;
    uint64_t open_ms_ = 0;
    uint64_t timeout_ms_ = 0;
    bool opened_ = false;
    bool has_video_ = false;
    bool has_audio_ = false;

    AVFormatContext *fmt_ctx_ = NULL;
    AVCodec         *audio_codec_ = NULL;
    AVCodec         *video_codec_ = NULL;
    AVCodecContext  *audio_cc_ = NULL;
    AVCodecContext  *video_cc_ = NULL;
    AVStream        *audio_stream_ = NULL;
    AVStream        *video_stream_ = NULL;
    AVDictionary    *opt_ = NULL;

    AVPixelFormat   pix_fmt_ = AV_PIX_FMT_YUV420P;
    int             width_ = 0;
    int             height_ = 0;
    int             gop_size_ = 0;
    int64_t         bit_rate_ = 0;
    int             video_frame_rate_ = 0;
    unsigned short  sps_len_ = 0;
    uint8_t        *sps_data_ = NULL;
    unsigned short  pps_len_ = 0;
    uint8_t        *pps_data_ = NULL;

    int64_t         video_base_pts_ = -1;
    int64_t         video_next_pts_ = 0;
    //AVFrame        *video_frame_ = NULL;

    AVSampleFormat audio_sample_fmt_ = AV_SAMPLE_FMT_NONE;
    int audio_bit_rate_        = 0;
    int audio_sample_rate_     = 0;
    int audio_channel_layout_  = 0;

    int audio_samples_count = 0;
    //AVFrame *audio_frame_= NULL;

    chef::task_thread *thread_ = NULL;

  private:
    // static constexpr enum AVCodecID AUDIO_CODEC_ID = AV_CODEC_ID_H264;
    // static constexpr enum AVCodecID VIDEO_CODEC_ID = AV_CODEC_ID_AAC;
    static constexpr int VIDEO_STREAM_INDEX = 0;
    static constexpr int AUDIO_STREAM_INDEX = 1;

}; // class Out

}; // namespace avkid
