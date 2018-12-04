#include "avkid_mix.h"

namespace avkid {

AVFrame *Mix::horizontal(AVFrame *left, AVFrame *right) {
  // TODO one nullptr one not.

  int dst_width = left->width; //left->width + right->width;
  int dst_height = left->height; //(left->height < right->height) ? left->height : right->height;

  AVFrame *dst = av_frame_alloc();
  dst->format = AV_PIX_FMT_YUV420P;
  dst->width = dst_width;
  dst->height = dst_height;
  dst->pts = left->pts; // TODO
  av_frame_get_buffer(dst, 32);

  memcpy(dst->data[0], left->data[0], dst_width * dst_height);
  memcpy(dst->data[1], left->data[1], dst_width * dst_height / 4);
  memcpy(dst->data[2], left->data[2], dst_width * dst_height / 4);

  return dst;
}

}
