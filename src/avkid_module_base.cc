#include "avkid_module_base.h"

namespace avkid {

ModuleBase::ModuleBase(bool async_mode)
  : async_mode_(async_mode)
{
  if (async_mode) {
    thread_ = std::make_shared<chef::task_thread>("avkid_base", chef::task_thread::RELEASE_MODE_DO_ALL_DONE);
    thread_->start();
  }
}

void PacketProducer::set_data_handler(PacketHandlerT ph) {
  ph_ = ph;
}

void FrameProducer::set_data_handler(FrameHandlerT fh) {
  fh_ = fh;
}

}
