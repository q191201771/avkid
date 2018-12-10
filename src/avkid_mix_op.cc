#include "avkid_mix_op.h"
#include "avkid.hpp"

namespace avkid {

AVFrame *MixOP::video_horizontal(AVFrame *left, AVFrame *right) {
  if (!left || !right) { return nullptr; }

  int dst_width = left->width * 2;
  int dst_height = left->height;

  AVFrame *dst = HelpOP::frame_alloc_prop();
  dst->format = AV_PIX_FMT_YUV420P;
  dst->width = dst_width;
  dst->height = dst_height;
  dst->pts = left->pts; // TODO
  dst->pkt_dts = left->pkt_dts;
  HelpOP::frame_alloc_buf(dst, false);

  memset(dst->data[0], 0, dst_width * dst_height);
  memset(dst->data[1], 0, dst_width * dst_height / 4);
  memset(dst->data[2], 0, dst_width * dst_height / 4);

  HelpOP::mix_video_pin_frame(dst, left, 0, 0);
  HelpOP::mix_video_pin_frame(dst, right, left->width, 0);

  return dst;
}

AVFrame *MixOP::audio(AVFrame *a, AVFrame *b) {
  if (!a || !b) { return nullptr; }

  // 开发时遇到一个奇怪的问题，origin数据format=floatp channels=2 nb_samples=1024，但是linesize[0]=8192，linesize[1]=0，
  // 按我的理解，linesize[0]=4096，linesize[1]=4096才是正常的
  // 在这里我们给目的frame设置好参数再alloc buf后，linesize[0]和linesize[1]都=4096
  // 并且查看origin里面的数据buf[0]和buf[1]的前4096个字节都有意义，buf[0]的后4096个字节都为0
  // 所以，只是单纯的decode后的linesize赋值错了吗？
  // 目前这里按nb_samples和channels去操作数据是正常的
  AVFrame *dst = HelpOP::frame_alloc_prop();
  dst->format = a->format;
  dst->channels = a->channels;
  dst->channel_layout = a->channel_layout;
  dst->nb_samples = a->nb_samples;
  dst->sample_rate = a->sample_rate;
  dst->pts = a->pts;
  dst->pkt_dts = a->pkt_dts;
  HelpOP::frame_alloc_buf(dst, true);

  int data_size = av_get_bytes_per_sample((enum AVSampleFormat)a->format);
  for (int i = 0; i < a->nb_samples; i++) {
    for (int ch = 0; ch < a->channels; ch++) {
      *(float *)(dst->data[ch] + data_size * i) = *(float *)(a->data[ch] + data_size * i) * 0.5f +
                                                  *(float *)(b->data[ch] + data_size * i) * 0.5f;
    }
  }

  return dst;
}

}
