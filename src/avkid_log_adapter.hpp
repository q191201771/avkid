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

#define AVKID_LOG_PACKET(packet, is_audio) if (packet) AVKID_LOG_DEBUG << "Packet " << (is_audio ? "A" : "V") << " pts:" << (packet)->pts << " dts:" << (packet)->dts << " duration:" << (packet)->duration << " size:" << (packet)->size << "\n";
#define AVKID_LOG_FRAME(frame, is_audio)   if (frame) AVKID_LOG_DEBUG << "Frame " << (is_audio ? "A" : "V") << " pts:" << (frame)->pts << " dts:" << (frame)->pkt_dts << " bets:" << (frame)->best_effort_timestamp <<  " duration:" << (frame)->pkt_duration << " kframe:" << (frame)->key_frame << "\n";
