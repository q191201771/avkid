#include <stdio.h>
#include <thread>
#include <deque>
#include "avkid.hpp"

using namespace avkid;

EncodePtr encode;
OutputPtr output;

class MixManager {
  public:
    void left_frame_cb(AVFrame *frame, bool is_audio) {
      if (is_audio) {
        encode->do_data(frame, is_audio);
        return;
      }

      if (right_frame_deque_.empty()) {
        AVFrame *rframe = HelpOP::frame_alloc_prop_ref_buf(frame);
        left_frame_deque_.push_back(rframe);
      } else {
        AVFrame *pair_frame = right_frame_deque_.front();
        right_frame_deque_.pop_front();
        AVFrame *dst = Mix::horizontal(frame, pair_frame);
        encode->do_data(dst, is_audio);
        HelpOP::frame_free_prop_unref_buf(&pair_frame);
        HelpOP::frame_free_prop_unref_buf(&dst);
      }
    }

    void right_frame_cb(AVFrame *frame, bool is_audio) {
      if (is_audio) {
        return;
      }

      if (left_frame_deque_.empty()) {
        AVFrame *rframe = HelpOP::frame_alloc_prop_ref_buf(frame);
        right_frame_deque_.push_back(rframe);
      } else {
        AVFrame *pair_frame = left_frame_deque_.front();
        left_frame_deque_.pop_front();
        AVFrame *dst = Mix::horizontal(pair_frame, frame);
        encode->do_data(dst, is_audio);
        HelpOP::frame_free_prop_unref_buf(&pair_frame);
        HelpOP::frame_free_prop_unref_buf(&dst);
      }
    }

  private:
    std::deque<AVFrame *> left_frame_deque_;
    std::deque<AVFrame *> right_frame_deque_;

};

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <left> <right> <duraton>\n";
    return -1;
  }
  std::string left = argv[1];
  std::string right = argv[2];
  int duration = atoi(argv[3]);

  HelpOP::global_init_ffmpeg();

  auto left_input = Input::create();
  auto left_decode = Decode::create();
  if (!left_input->open(left)) {
    AVKID_LOG_ERROR << "Open " << left << " failed.\n";
    return -1;
  }
  left_decode->open(left_input->in_fmt_ctx());

  auto right_input = Input::create();
  auto right_decode = Decode::create();
  if (!right_input->open(right)) {
    AVKID_LOG_ERROR << "Open " << right << " failed.\n";
    return -1;
  }
  right_decode->open(right_input->in_fmt_ctx());

  encode = Encode::create();
  encode->open(left_input->in_fmt_ctx(), left_input->video_width() * 2, left_input->video_height());
  output = Output::create();
  output->open("out_mix.flv", left_input->in_fmt_ctx(), left_input->video_width() * 2, left_input->video_height());

  std::shared_ptr<MixManager> mm = std::make_shared<MixManager>();

  combine(left_input, left_decode);
  AVKID_COMBINE_MODULE_CC(left_decode, mm, &MixManager::left_frame_cb);
  combine(right_input, right_decode);
  AVKID_COMBINE_MODULE_CC(right_decode, mm, &MixManager::right_frame_cb);
  combine(encode, output);

  std::thread t1([&]{
    left_input->read(duration * 1000);
  });

  std::thread t2([&]{
    right_input->read(duration * 1000);
  });

  t1.join();
  t2.join();

  return 0;
}
