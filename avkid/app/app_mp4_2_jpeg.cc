#include <stdio.h>
#include <unistd.h>

#include <avkid.hpp>
#include <chef_base/chef_filepath_op.hpp>

using namespace avkid;

static const int RETRY_NUM = 3;
static const int RETRY_INTERVAL_MS = 2000;

static const int APP_RET_NO_ERROR          = 0;
static const int APP_RET_PARAM_ERROR       = -1;
static const int APP_RET_OUTPUT_DIR_ERROR  = -2;
static const int APP_RET_INPUT_OPEN_ERROR  = -3;
static const int APP_RET_INPUT_READ_ERROR  = -4;
static const int APP_RET_OPEN_DECODE_ERROR = -5;

std::string g_in_url;
std::string g_output_dir;
int g_width;
int g_height;
int g_interval_ms;

std::shared_ptr<Input> g_input;
int g_jpeg_count = 0;
int64_t g_prev_video_duration_ms = -1;

static std::string jpeg_filename() {
  std::string filename = chef::strings_op::string_printf("%d.jpeg", ++g_jpeg_count);
  return chef::filepath_op::join(g_output_dir, filename);
}

class DumpJpeg : public FrameConsumerInterface {
  public:
    virtual void do_data(AVFrame *frame, bool is_audio) {
      if (is_audio) { return; }

      if (g_prev_video_duration_ms == -1 || g_input->video_duration_ms() - g_prev_video_duration_ms >= g_interval_ms) {
        if (g_width == 0 || g_height == 0) {
          HelpOP::dump_mjpeg(frame, jpeg_filename());
        } else {
          AVFrame *dst_frame = HelpOP::scale_video_frame(frame, g_width, g_height);
          HelpOP::dump_mjpeg(dst_frame, jpeg_filename());
          HelpOP::frame_free_prop_unref_buf(&dst_frame);
        }

        g_prev_video_duration_ms = g_input->video_duration_ms();
      }
    }
};

int main(int argc, char **argv) {
  uint64_t bt = chef::stuff_op::tick_msec();

  int ret;

  for (int i = 0; i < RETRY_NUM; i++){
    if (argc < 6) {
      std::cerr << "Usage: " << argv[0] << " <mp4 url> <output dir> <width:set 0 to copy> <height:set 0 to copy> <interval ms>\n";
      ret = APP_RET_PARAM_ERROR;
      break;
    }

    g_in_url      = argv[1];
    g_output_dir  = argv[2];
    g_width       = atoi(argv[3]);
    g_height      = atoi(argv[4]);
    g_interval_ms = atoi(argv[5]);
    AVKID_LOG_INFO << "App param. in_url:" << g_in_url << ", output_dir:" << g_output_dir << ", g_width:" << g_width
                   << ", g_height:" << g_height << ", g_interval_ms:" << g_interval_ms << "\n";

    if (chef::filepath_op::mkdir_recursive(g_output_dir) == -1) {
      AVKID_LOG_ERROR << "Init output dir failed. output_dir:" << g_output_dir << "\n";
      usleep(RETRY_INTERVAL_MS * 1000);
      ret = APP_RET_OUTPUT_DIR_ERROR;
      continue;
    }

    HelpOP::global_init_ffmpeg(false);

    g_input = Input::create();
    auto decode = Decode::create();
    auto dump_jpeg = std::make_shared<DumpJpeg>();

    combine(g_input, decode, dump_jpeg);

    if (!g_input->open(g_in_url)) {
      AVKID_LOG_ERROR << "Open " << g_in_url << " failed.\n";
      usleep(RETRY_INTERVAL_MS * 1000);
      ret = APP_RET_INPUT_OPEN_ERROR;
      continue;
    }

    if (!decode->open(g_input->in_fmt_ctx())) {
      AVKID_LOG_ERROR << "Open decode failed.\n";
      usleep(RETRY_INTERVAL_MS * 1000);
      ret = APP_RET_OPEN_DECODE_ERROR;
      continue;
    }

    if (!g_input->read()) {
      AVKID_LOG_ERROR << "Read input failed.\n";
      usleep(RETRY_INTERVAL_MS * 1000);
      ret = APP_RET_INPUT_READ_ERROR;
      continue;
    }

    ret = APP_RET_NO_ERROR;
    break;
  } while(0);

  std::cerr << "Done. cost:" << chef::stuff_op::tick_msec() - bt << "ms, jpeg count:" << g_jpeg_count << "\n";
  std::cerr << "result:" << ret << " " << g_jpeg_count << "\n";
  return ret;
}
