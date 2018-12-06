/**
 * @file   avkid_mix.h
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

class Mix {
  public:
    // 水平合并，要求左右两幅图的宽高相等，合并后高不变，宽为二倍
    static AVFrame *horizontal(AVFrame *left, AVFrame *right);
};

}
