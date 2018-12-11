#include <stdio.h>
#include <thread>
#include <queue>
#include "avkid.hpp"

using namespace avkid;

class MixManager : public FrameProducer {
  public:
    void left_frame_cb(AVFrame *frame, bool is_audio) {
      if (is_audio) {
        frame_handler(frame, is_audio);
        //AVKID_LOG_FRAME(frame, is_audio);

        //if (right_audio_queue_.empty()) {
        //  AVFrame *rframe = HelpOP::frame_alloc_copy_prop_ref_buf(frame);
        //  left_audio_queue_.push_back(rframe);
        //} else {
        //  AVFrame *pair_frame = right_audio_queue_.front();
        //  right_audio_queue_.pop_front();
        //  AVFrame *dst = MixOP::audio(frame, pair_frame);

        //  frame_handler(dst, is_audio);
        //  HelpOP::frame_free_prop_unref_buf(&pair_frame);
        //  HelpOP::frame_free_prop_unref_buf(&dst);
        //}

        return;
      }

      if (right_video_queue_.empty()) {
        AVFrame *rframe = HelpOP::frame_alloc_copy_prop_ref_buf(frame);
        left_video_queue_.push(rframe);
      } else {
        AVFrame *pair_frame = right_video_queue_.front();
        right_video_queue_.pop();
        AVFrame *dst = MixOP::video_horizontal(frame, pair_frame);

        frame_handler(dst, is_audio);
        HelpOP::frame_free_prop_unref_buf(&pair_frame);
        HelpOP::frame_free_prop_unref_buf(&dst);
      }
    }

    void right_frame_cb(AVFrame *frame, bool is_audio) {
      if (is_audio) {
        //if (left_audio_deque_.empty()) {
        //  AVFrame *rframe = HelpOP::frame_alloc_copy_prop_ref_buf(frame);
        //  left_audio_deque_.push_back(rframe);
        //} else {
        //  AVFrame *pair_frame = left_audio_deque_.front();
        //  left_audio_deque_.pop_front();
        //  AVFrame *dst = MixOP::audio(pair_frame, frame);

        //  frame_handler(dst, is_audio);
        //  HelpOP::frame_free_prop_unref_buf(&pair_frame);
        //  HelpOP::frame_free_prop_unref_buf(&dst);
        //}

        return;
      }

      if (left_video_queue_.empty()) {
        AVFrame *rframe = HelpOP::frame_alloc_copy_prop_ref_buf(frame);
        right_video_queue_.push(rframe);
      } else {
        AVFrame *pair_frame = left_video_queue_.front();
        left_video_queue_.pop();
        AVFrame *dst = MixOP::video_horizontal(pair_frame, frame);

        frame_handler(dst, is_audio);
        HelpOP::frame_free_prop_unref_buf(&pair_frame);
        HelpOP::frame_free_prop_unref_buf(&dst);
      }
    }

  private:
    std::queue<AVFrame *> left_video_queue_;
    std::queue<AVFrame *> right_video_queue_;
    std::queue<AVFrame *> left_audio_queue_;
    std::queue<AVFrame *> right_audio_queue_;

};

int main(int argc, char **argv) {
  uint64_t bt = chef::stuff_op::tick_msec();
  {
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

    auto encode = Encode::create(true);
    encode->open(left_input->in_fmt_ctx(), left_input->video_width() * 2, left_input->video_height());
    auto output = Output::create();
    output->open("out_mix.flv", left_input->in_fmt_ctx(), left_input->video_width() * 2, left_input->video_height());

    std::shared_ptr<MixManager> mm = std::make_shared<MixManager>();

    combine(left_input, left_decode);
    AVKID_COMBINE_MODULE_CC(left_decode, mm, &MixManager::left_frame_cb);

    combine(right_input, right_decode);
    AVKID_COMBINE_MODULE_CC(right_decode, mm, &MixManager::right_frame_cb);

    combine(combine(mm, encode), output);

    std::thread t1([&]{
      left_input->read(duration * 1000);
    });

    std::thread t2([&]{
      right_input->read(duration * 1000);
    });

    t1.join();
    t2.join();
  }
  std::cerr << "Cost:" << chef::stuff_op::tick_msec() - bt << "\n";
  return 0;
}
