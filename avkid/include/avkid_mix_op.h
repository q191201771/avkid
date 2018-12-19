/**
 * @file   avkid_mix_op.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class MixOP {
  public:
    // 水平合并，要求左右两幅图的宽高相等，合并后高不变，宽为二倍
    static AVFrame *video_horizontal(AVFrame *left, AVFrame *right);

    // @TODO 目前要求两路音频格式完全相同，并且和输出音频设置的格式也完全相同
    static AVFrame *audio(AVFrame *a, AVFrame *b);
};

}
