/**
 * @file   avkid_mix.h
 * @author chef
 *
 */

#pragma once

#include "avkid_common.hpp"

namespace avkid {

class Mix {
  AVFrame *horizontal(AVFrame *left, AVFrame *right);
};

}
