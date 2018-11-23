#include <stdio.h>
#include <thread>
#include "avkid_in.h"
#include "avkid_out.h"
#include "avkid_log_adapter.hpp"
#include "chef_task_thread.hpp"

using namespace avkid;

std::string g_in_url;
int  g_record_sec;
bool g_record_audio;
bool g_record_video;
bool g_transcode;

In *g_in;
Out *g_out;

std::string produce_filename() {
  char buf[128] = {0};
  snprintf(buf, 127, "./resource/out_rtmp2flv_%llu_a%d_v%d_t%d.flv", chef::stuff_op::unix_timestamp_msec(), g_record_audio, g_record_video, g_transcode);
  return std::string(buf);
}

int main(int argc, char **argv) {
  if (argc != 6) {
    AVKID_LOG_ERROR << "Usage: %s <rtmp url> <record sec> <record audio> <record video> <transcode>\n";
    return -1;
  }

  global_init_ffmpeg();

  g_in_url = argv[1];
  g_record_sec = atoi(argv[2]);
  g_record_audio = atoi(argv[3]);
  g_record_video = atoi(argv[4]);
  g_transcode = atoi(argv[5]);


  g_out = new Out(g_transcode ? Out::INPUT_TYPE_AV_FRAME : Out::INPUT_TYPE_AV_PACKET);

  g_in = new In(g_out, true);
  if (!g_in->open(g_in_url)) {
    AVKID_LOG_ERROR << "open " << g_in_url << " failed.\n";
    return -1;
  }

  if (g_record_audio) {
    g_out->init_audio(g_in->audio_dec_ctx()->sample_fmt,
                      g_in->audio_dec_ctx()->bit_rate,
                      g_in->audio_dec_ctx()->sample_rate,
                      g_in->audio_dec_ctx()->channel_layout);
  }

  if (g_record_video) {
    g_out->init_video(g_in->video_dec_ctx()->pix_fmt,
                      g_in->video_dec_ctx()->width,
                      g_in->video_dec_ctx()->height,
                      g_in->video_dec_ctx()->gop_size,
                      g_in->video_dec_ctx()->bit_rate,
                      g_in->video_frame_rate(),
                      g_in->sps_len(),
                      g_in->sps_data(),
                      g_in->pps_len(),
                      g_in->pps_data());
  }

  if (!g_out->open(produce_filename())) {
    assert(0);
  }

  std::thread t([] {
    g_in->read();
  });

  sleep(g_record_sec);
  g_in->stop_read();
  t.join();

  delete g_in;
  delete g_out;
  AVKID_LOG_DEBUG << "done.\n";
  sleep(-1);

  return 0;
}
