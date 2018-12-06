#include "avkid_mix.h"
#include "avkid.hpp"

namespace avkid {

AVFrame *Mix::horizontal(AVFrame *left, AVFrame *right) {
  // TODO one nullptr one not.

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

}
