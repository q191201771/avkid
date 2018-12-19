/**
 * @file   avkid_combine.hpp
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

template <typename DstT, typename DataT>
class Broadcast {
  public:
    void add_data_handler(std::shared_ptr<DstT> dst) {
      dst_list_.insert(dst);
    }
    void del_data_handler(std::shared_ptr<DstT> dst) {
      dst_list_.erase(dst);
    }

    void do_data(DataT *data, bool is_bool) {
      for (auto dst : dst_list_) {
        dst->do_data(data, is_bool);
      }
    }

  private:
    std::unordered_set<std::shared_ptr<DstT> > dst_list_;

  public:
    Broadcast() {}
  private:
    Broadcast(const Broadcast &) = delete;
    Broadcast operator=(const Broadcast &) = delete;
};

typedef Broadcast<PacketConsumerInterface, AVPacket> PacketBroadcast;
typedef Broadcast<FrameConsumerInterface, AVFrame>   FrameBroadcast;

}
