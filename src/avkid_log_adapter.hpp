/**
 * @file   avkid_log_adapter.hpp
 * @author chef
 *
 */

#pragma once

#include <iostream>
#include "chef_stuff_op.hpp"

#define AVKID_LOG_DEBUG std::cout << "DEBUG " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "
#define AVKID_LOG_INFO  std::cout << "INFO  " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "
#define AVKID_LOG_WARN  std::cout << "WARN  " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "
#define AVKID_LOG_ERROR std::cout << "ERROR " << chef::stuff_op::tick_msec() << " " << __FILE__ << ":" << __LINE__ << " "

#define FFMPEG_FAILED_LOG(funcname, ret) AVKID_LOG_ERROR << funcname << " failed. " << stringify_ffmpeg_error(ret) << "\n";

#define RETURN_FALSE_IF_NULL(x, funcname) if (!x) { AVKID_LOG_ERROR << funcname << "failed.\n"; return false; }
#define RETURN_FALSE_IF_FFMPEG_FAILED(funcname, ret) if (ret != 0) { FFMPEG_FAILED_LOG(funcname, ret); return false; }
