// Copyright 2022 Sony Group Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LOG_HPP_
#define LOG_HPP_

#include <rcutils/logging_macros.h>
#include <sstream>

#define rcl_content_filter_fallback_log_(level, cat, msg) \
  { \
    std::stringstream log_ss_tmp__; \
    log_ss_tmp__ << msg; \
    RCUTILS_LOG_ ## level ## _NAMED(#cat, "%s", log_ss_tmp__.str().c_str()); \
  }

#define logDebug(cat, msg) rcl_content_filter_fallback_log_(DEBUG, cat, msg)
#define logInfo(cat, msg) rcl_content_filter_fallback_log_(INFO, cat, msg)
#define logWarning(cat, msg) rcl_content_filter_fallback_log_(WARN, cat, msg)
#define logError(cat, msg) rcl_content_filter_fallback_log_(ERROR, cat, msg)

#endif  // LOG_HPP_
