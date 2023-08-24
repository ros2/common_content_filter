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

#ifndef RCL_CONTENT_FILTER_FALLBACK__RCL_CONTENT_FILTER_FALLBACK_H_
#define RCL_CONTENT_FILTER_FALLBACK__RCL_CONTENT_FILTER_FALLBACK_H_

#include <rcutils/allocator.h>
#include <rmw/subscription_content_filter_options.h>
#include <rosidl_runtime_c/message_type_support_struct.h>

#include "rcl_content_filter_fallback/visibility_control.h"


#ifdef __cplusplus
extern "C"
{
#endif

/// Create a rcl content filter fallback instance to filter ros 2 message data.
/**
 * \param[in] type_support Type support of the topic data being filtered.
 * \return a valid address if success, or NULL on failure.
 */
RCL_CONTENT_FILTER_FALLBACK_PUBLIC
void *
rcl_content_filter_fallback_create(const rosidl_message_type_support_t * type_support);

/// Check if the rcl content filter fallback instance is enabled.
/**
 * \param[in] instance A rcl content filter fallback instance.
 * \return true if enabled, or false.
 */
RCL_CONTENT_FILTER_FALLBACK_PUBLIC
bool
rcl_content_filter_fallback_is_enabled(void * instance);

/// Use the rcl content filter fallback instance to evalute the data.
/**
 * \param[in] instance A rcl content filter fallback instance.
 * \param[in] data ros 2 payload data.
 * \param[in] serialized Indicate whether the ros 2 payload data is serialized or not.
 * \return true if evaluate successfully, or false.
 */
RCL_CONTENT_FILTER_FALLBACK_PUBLIC
bool
rcl_content_filter_fallback_evaluate(void * instance, void * data, bool serialized);

/// Set a rcl content filter fallback instance with an options.
/**
 * \param[in] instance A rcl content filter fallback instance.
 * \param[in] options A content filter options.
 * \return true if success, or false.
 */
RCL_CONTENT_FILTER_FALLBACK_PUBLIC
bool
rcl_content_filter_fallback_set(
  void * instance,
  const rmw_subscription_content_filter_options_t * options
);

/// Get the content filter options from a rcl content filter fallback instance.
/**
 * \param[in] instance A rcl content filter fallback instance.
 * \param[in] allocator A valid allocator.
 * \param[in] options a filter options.
 * \return true if success, or false.
 */
RCL_CONTENT_FILTER_FALLBACK_PUBLIC
bool
rcl_content_filter_fallback_get(
  void * instance,
  rcutils_allocator_t * allocator,
  rmw_subscription_content_filter_options_t * options
);

/// Destroy the content filter instance.
/**
 * \param[in] instance A content filter instance.
 */
RCL_CONTENT_FILTER_FALLBACK_PUBLIC
void
rcl_content_filter_fallback_destroy(void * instance);

#ifdef __cplusplus
}
#endif

#endif  // RCL_CONTENT_FILTER_FALLBACK__RCL_CONTENT_FILTER_FALLBACK_H_
