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

#include "rcl_content_filter_fallback/rcl_content_filter_fallback.h"

#include <rcutils/allocator.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

#include <rmw/subscription_content_filter_options.h>
#include <rosidl_typesupport_introspection_c/identifier.h>
#include <rosidl_typesupport_introspection_c/message_introspection.h>

#include <functional>
#include <mutex>

#include <rosidl_typesupport_introspection_cpp/identifier.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>

#include <tao/pegtl.hpp>

#include "FilterFactory.hpp"
#include "Log.hpp"
#include "Utilities.hpp"

namespace rcl_content_filter_fallback
{

using FilterFactory = rcl_content_filter_fallback::SQLFilter::FilterFactory;
using IContentFilter = rcl_content_filter_fallback::IContentFilter;

FilterFactory *
get_rcl_content_filter_fallback_factory()
{
  static FilterFactory content_filter_factory;
  return &content_filter_factory;
}

template<typename MembersType, typename MessageInitialization>
auto
get_message(
  const rosidl_message_type_support_t * type_support_introspection,
  MessageInitialization message_initialization)
{
  const MembersType * members =
    static_cast<const MembersType *>(type_support_introspection->data);
  if (!members) {
    throw std::runtime_error("The data in the type support introspection is invalid.");
  }

  auto msg = std::unique_ptr<void, std::function<void(void *)>>(
    malloc(members->size_of_),
    [members](void * msg_ptr) {
      members->fini_function(msg_ptr);
      free(msg_ptr);
    });
  if (msg) {
    members->init_function(msg.get(), message_initialization);
  }

  return msg;
}

auto get_message_buffer(const rosidl_message_type_support_t * type_support)
{
  const rosidl_message_type_support_t * type_support_introspection =
    get_type_support_introspection(type_support);
  if (!type_support_introspection) {
    throw std::runtime_error("failed to get type support introspection");
  }

  if (type_support_introspection->typesupport_identifier ==
    rosidl_typesupport_introspection_c__identifier)
  {
    return get_message<rosidl_typesupport_introspection_c__MessageMembers>(
      type_support_introspection, ROSIDL_RUNTIME_C_MSG_INIT_ZERO);
  } else {
    return get_message<rosidl_typesupport_introspection_cpp::MessageMembers>(
      type_support_introspection, rosidl_runtime_cpp::MessageInitialization::ZERO);
  }
}

class ContentFilterWrapper
{
public:
  explicit ContentFilterWrapper(const rosidl_message_type_support_t * type_support)
  : type_support_(type_support)
  {}

  ~ContentFilterWrapper()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (filter_instance_) {
      FilterFactory::ReturnCode_t ret =
        get_rcl_content_filter_fallback_factory()->delete_content_filter(filter_instance_);
      if (ret != FilterFactory::RETCODE_OK) {
        logError(SQLFILTER, "Failed to delete content filter: " << ret);
      }

      filter_instance_ = nullptr;
    }
  }

  bool evaluate(void * ros_data, bool serialized)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!filter_instance_) {
      logWarning(SQLFILTER, "Common content filter is not set");
      return true;
    }

    if (serialized) {
      const rmw_serialized_message_t * serialized_message =
        (const rmw_serialized_message_t *)ros_data;
      if (!deserialized_buffer_) {
        deserialized_buffer_ = get_message_buffer(type_support_);
      }
      rmw_ret_t rmw_ret =
        rmw_deserialize(serialized_message, type_support_, deserialized_buffer_.get());
      ros_data = deserialized_buffer_.get();
      if (rmw_ret != RMW_RET_OK) {
        logError(SQLFILTER, "Failed to deserialize message");
        return false;
      }
    }

    return filter_instance_->evaluate(ros_data);
  }

  bool set_filter_expression(
    const std::string & filter_expression,
    const std::vector<std::string> & expression_parameters)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    const char * tip = (filter_instance_ == nullptr) ? "create" : "set";
    FilterFactory::ReturnCode_t ret =
      get_rcl_content_filter_fallback_factory()->create_content_filter(
      type_support_,
      filter_expression.c_str(),
      expression_parameters,
      filter_instance_);
    if (ret != FilterFactory::RETCODE_OK) {
      logError(SQLFILTER, "failed to " << tip << " content filter, error code: " << ret);
      return false;
    }

    filter_expression_ = filter_expression;
    expression_parameters_ = expression_parameters;

    return true;
  }

  bool get_filter_expression(
    std::string & filter_expression,
    std::vector<std::string> & expression_parameters)
  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!filter_instance_) {
      logError(SQLFILTER, "content filter instance is not created");
      return false;
    }

    filter_expression = filter_expression_;
    expression_parameters = expression_parameters_;

    return true;
  }

  IContentFilter * filter_instance()
  {
    return filter_instance_;
  }

  bool is_enabled()
  {
    return filter_instance_ != nullptr;
  }

private:
  const rosidl_message_type_support_t * type_support_;
  std::unique_ptr<void, std::function<void(void *)>> deserialized_buffer_ = nullptr;
  IContentFilter * filter_instance_ = nullptr;
  std::string filter_expression_;
  std::vector<std::string> expression_parameters_;
  std::mutex mutex_;
};


rcl_content_filter_fallback::ContentFilterWrapper * validate(void * instance)
{
  auto content_filter_wrapper =
    static_cast<rcl_content_filter_fallback::ContentFilterWrapper *>(instance);
  if (!content_filter_wrapper) {
    logError(SQLFILTER, "Invalid instance");
    return nullptr;
  }
  return content_filter_wrapper;
}

}  // namespace rcl_content_filter_fallback


#ifdef __cplusplus
extern "C"
{
#endif

void *
rcl_content_filter_fallback_create(const rosidl_message_type_support_t * type_support)
{
  return new rcl_content_filter_fallback::ContentFilterWrapper(type_support);
}

bool
rcl_content_filter_fallback_is_enabled(void * instance)
{
  rcl_content_filter_fallback::ContentFilterWrapper * content_filter_wrapper =
    rcl_content_filter_fallback::validate(instance);
  if (!content_filter_wrapper) {
    return false;
  }

  return content_filter_wrapper->is_enabled();
}

bool
rcl_content_filter_fallback_evaluate(void * instance, void * ros_data, bool serialized)
{
  rcl_content_filter_fallback::ContentFilterWrapper * content_filter_wrapper =
    rcl_content_filter_fallback::validate(instance);
  if (!content_filter_wrapper) {
    return false;
  }

  if (!ros_data) {
    logError(SQLFILTER, "Invalid arguments");
    return false;
  }

  bool ret = false;
  try {
    ret = content_filter_wrapper->evaluate(ros_data, serialized);
  } catch (const std::runtime_error & e) {
    logError(SQLFILTER, "Failed to evaluate: " << e.what());
  }

  return ret;
}

bool
rcl_content_filter_fallback_set(
  void * instance,
  const rmw_subscription_content_filter_options_t * options
)
{
  rcl_content_filter_fallback::ContentFilterWrapper * content_filter_wrapper =
    rcl_content_filter_fallback::validate(instance);
  if (!content_filter_wrapper) {
    return false;
  }

  if (!options) {
    logError(SQLFILTER, "Invalid arguments");
    return false;
  }

  std::vector<std::string> expression_parameters;
  for (size_t i = 0; i < options->expression_parameters.size; ++i) {
    expression_parameters.push_back(options->expression_parameters.data[i]);
  }

  bool ret = false;
  try {
    ret = content_filter_wrapper->set_filter_expression(
      options->filter_expression, expression_parameters
    );
  } catch (const std::runtime_error & e) {
    logError(SQLFILTER, "Failed to create content filter: " << e.what());
  }

  return ret;
}

bool
rcl_content_filter_fallback_get(
  void * instance,
  rcutils_allocator_t * allocator,
  rmw_subscription_content_filter_options_t * options
)
{
  rcl_content_filter_fallback::ContentFilterWrapper * content_filter_wrapper =
    rcl_content_filter_fallback::validate(instance);
  if (!content_filter_wrapper) {
    return false;
  }

  if (!allocator || !options) {
    logError(SQLFILTER, "Invalid arguments");
    return false;
  }

  std::string filter_expression;
  std::vector<std::string> expression_parameters;
  bool ret = content_filter_wrapper->get_filter_expression(
    filter_expression,
    expression_parameters
  );
  if (!ret) {
    return false;
  }

  std::vector<const char *> string_array;
  for (size_t i = 0; i < expression_parameters.size(); ++i) {
    string_array.push_back(expression_parameters[i].c_str());
  }

  rmw_ret_t rmw_ret = rmw_subscription_content_filter_options_set(
    filter_expression.c_str(),
    string_array.size(),
    string_array.data(),
    allocator,
    options
  );

  if (rmw_ret != RMW_RET_OK) {
    logError(SQLFILTER, rmw_get_error_string().str);
    rmw_reset_error();
    return false;
  }

  return true;
}

void
rcl_content_filter_fallback_destroy(void * instance)
{
  rcl_content_filter_fallback::ContentFilterWrapper * content_filter_wrapper =
    rcl_content_filter_fallback::validate(instance);
  if (!content_filter_wrapper) {
    return;
  }

  delete content_filter_wrapper;
}

#ifdef __cplusplus
}
#endif
