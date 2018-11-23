/**
 * @file   avkid_in.h
 * @author chef
 *
 */

#pragma once

#include <string>
#include "avkid_common.hpp"
#include "chef_snippet.hpp"

namespace avkid {

class InObserver {
  public:
    virtual ~InObserver() {}

    // 未解码数据
    virtual void packet_cb(AVPacket *av_packet, bool is_audio) = 0;
    // 已解码数据
    virtual void frame_cb(AVFrame *av_frame, bool is_audio) = 0;
};

class In {
  public:
    In(InObserver *observer, bool should_decode) : observer_(observer), should_decode_(should_decode) {}
    ~In();

    /**
     * @brief 非阻塞函数，在read前调用，且一个对象只允许调用一次
     *
     * @param url 播放地址，目前支持mp4文件以及rtmp流，视频h264格式，音频AAC格式
     * @param timeout_ms 打开超时时间，如果为0，则使用底层默认超时时间
     *
     */
    bool open(const std::string &url, uint64_t timeout_ms=0);

    /**
     * @brief 阻塞函数，直到文件读完或流断了
     *
     */
    bool read();

    // 关闭读，调用stop_read后，read函数会结束阻塞并返回
    void stop_read() { stop_flag_ = true; }

  public:
    // open成功后外部可以拿AVCodecContext获取一些编码信息，比如宽、高等
    CHEF_PROPERTY_WITH_INIT_VALUE(AVCodecContext *, audio_dec_ctx, NULL);
    CHEF_PROPERTY_WITH_INIT_VALUE(AVCodecContext *, video_dec_ctx, NULL);
    CHEF_PROPERTY_WITH_INIT_VALUE(int, video_frame_rate, 0);
    CHEF_PROPERTY_WITH_INIT_VALUE(unsigned short, sps_len, 0);
    CHEF_PROPERTY_WITH_INIT_VALUE(unsigned short, pps_len, 0);
    CHEF_PROPERTY_WITH_INIT_VALUE(uint8_t *, sps_data, NULL);
    CHEF_PROPERTY_WITH_INIT_VALUE(uint8_t *, pps_data, NULL);

  private:
    static int interrupt_cb(void *opaque);

  private:
    In(const In &);
    In &operator=(const In &);

  private:
    std::string url_ ;
    uint64_t    open_ms_ = 0;
    uint64_t    timeout_ms_ = 0;
    bool        opened_ = false;

    AVFormatContext      *fmt_ctx_ = NULL;
    unsigned int          audio_stream_index_ = 0;
    unsigned int          video_stream_index_ = 0;
    AVCodecParserContext *audio_parser_ = NULL;
    AVCodecParserContext *video_parser_ = NULL;

    int           video_frame_count_ = 0;
    int           audio_frame_count_ = 0;

    InObserver *observer_ = NULL;
    bool should_decode_ = false;

    bool stop_flag_ = false;

}; // class In

}; // namespace avkid
