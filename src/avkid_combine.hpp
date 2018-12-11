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
std::shared_ptr<ConsumerT> combine(std::shared_ptr<ProducerT> in, std::shared_ptr<ConsumerT> out) {
  AVKID_COMBINE_MODULE_CC(in, out, &ConsumerT::do_data);
  return out;
}

}
