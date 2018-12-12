/**
 * @file   avkid_log_adapter.hpp
 * @author chef
 *
 */

#pragma once

#include <iostream>
#include "chef_stuff_op.hpp"
#include "avkid_help_op.h"

#define AVKID_LOG_DEBUG std::cout << "DEBUG " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "
#define AVKID_LOG_INFO  std::cout << "INFO  " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "
#define AVKID_LOG_WARN  std::cout << "WARN  " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "
#define AVKID_LOG_ERROR std::cout << "ERROR " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "

#define AVKID_LOG_FFMPEG_ERROR(iret) AVKID_LOG_ERROR << HelpOP::stringify_ffmpeg_error(iret) << "\n";
#define FFMPEG_FAILED_LOG(funcname, ret) AVKID_LOG_ERROR << funcname << " failed. " << HelpOP::stringify_ffmpeg_error(ret) << "\n";

#define AVKID_LOG_PACKET(packet, is_audio) \
if (packet) { \
  std::ostringstream oss; \
oss << "Packet " << (is_audio ? "A" : "V") << " size:" << (packet)->size; \
oss << " pts:" << (packet)->pts << " dts:" << (packet)->dts << " duration:" << (packet)->duration; \
  AVKID_LOG_INFO << oss.str() << "\n"; \
}

#define AVKID_LOG_FRAME(frame, is_audio) \
if (frame) { \
  std::ostringstream oss; \
oss << "Frame " << (is_audio ? "A" : "V") << " linesize:(" << (frame)->linesize[0] << " " << (frame)->linesize[1] << " " << (frame)->linesize[2] << ")"; \
oss << " pts:" << (frame)->pts << " dts:" << (frame)->pkt_dts << " duration:" << (frame)->pkt_duration; \
oss << " format:" << av_get_sample_fmt_name((enum AVSampleFormat)(frame)->format); \
  if (is_audio) { \
oss << " nb_samples:" << (frame)->nb_samples << " sample_rate:" << (frame)->sample_rate << " channel_layout:" << (frame)->channel_layout; \
oss << " channels:" << (frame)->channels; \
  } else { \
oss << " width:" << (frame)->width << " height:" << (frame)->height << " format:" << av_get_sample_fmt_name((enum AVSampleFormat)(frame)->format); \
oss << " key_frame:" << (frame)->key_frame; \
  } \
  AVKID_LOG_INFO << oss.str() << "\n"; \
}

