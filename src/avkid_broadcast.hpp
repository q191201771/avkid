/**
 * @file   avkid_combine.hpp
 * @author chef
 *
 */

#pragma once

#include "avkid_fwd.hpp"

namespace avkid {

template <typename SrcT, typename DstT, typename DataT>
class Broadcast {
  public:
    void set_src(std::shared_ptr<SrcT> src) {
      src_ = src;
    }

    void add_listener(std::shared_ptr<DstT> dst) {
      dst_list_.insert(dst);
    }
    void del_listener(std::shared_ptr<DstT> dst) {
      dst_list_.erase(dst);
    }

    void do_data(DataT *data, bool is_bool) {
      for (auto dst : dst_list_) {
        dst->do_data(data, is_bool);
      }
    }

  private:
    std::shared_ptr<SrcT> src_;
    std::unordered_set<std::shared_ptr<DstT> > dst_list_;
};

typedef Broadcast<Input, PacketConsumerInterface, AVPacket>  InputBroadcast;
typedef Broadcast<Decode, FrameConsumerInterface, AVFrame>   DecodeBroadcast;
typedef Broadcast<Filter, FrameConsumerInterface, AVFrame>   FilterBroadcast;
typedef Broadcast<Encode, PacketConsumerInterface, AVPacket> EncodeBroadcast;

}
