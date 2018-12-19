/**
 * @file   avkid_combine.hpp
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

#define AVKID_COMBINE_MODULE_CC(in, out, func) (in)->set_data_handler(std::bind(func, out, std::placeholders::_1, std::placeholders::_2));
#define AVKID_COMBINE_MODULE_C(in, func)       (in)->set_data_handler(std::bind(func, std::placeholders::_1, std::placeholders::_2));

template <typename ProducerT, typename ConsumerT>
void combine(std::shared_ptr<ProducerT> in, std::shared_ptr<ConsumerT> out) {
  AVKID_COMBINE_MODULE_CC(in, out, &ConsumerT::do_data);
  //return out;
}

template <typename First, typename T, typename... Args>
void combine(First first, const T &val, const Args&... args) {
  combine(first, val);
  combine(val, args...);
}

template <typename First, typename ...Args>
void combine(First first, Args... args) {
  if (sizeof...(args) == 0) { return; }

  combine(first, args...);
}

}
