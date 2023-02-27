// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file FilterField.cpp
 */

#include "FilterField.hpp"

#include <rosidl_runtime_c/string.h>
#include <rosidl_typesupport_introspection_c/identifier.h>
#include <rosidl_typesupport_introspection_c/message_introspection.h>

#include <cassert>
#include <string>
#include <unordered_set>
#include <vector>
#include <rosidl_typesupport_introspection_cpp/identifier.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>

#include "FilterPredicate.hpp"
#include "FilterValue.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{


template<typename MembersType>
bool
FilterField::get_msg_data_address(
  const void * untype_members,
  FieldAccessor & accessor,
  const void * & data)
{
  const MembersType * members = static_cast<const MembersType *>(untype_members);
  if (!members) {
    return false;
  }

  const auto member = members->members_ + accessor.member_index;

  uint64_t addr = reinterpret_cast<uint64_t>(data);
  if (member->is_array_) {
    size_t array_size = member->array_size_;
    if (array_size == 0) {
      array_size = member->size_function(
        reinterpret_cast<void *>(addr + member->offset_));
    }

    if (accessor.array_index >= array_size) {
      return false;
    }

    data = member->get_function(
      reinterpret_cast<void *>(addr + member->offset_), accessor.array_index);
  } else {
    data = reinterpret_cast<void *>(addr + member->offset_);
  }

  return true;
}

bool FilterField::set_value(
  const void * data,
  size_t n)
{
  bool last_step = access_path_.size() - 1 == n;
  bool ret = false;
  bool is_c_type_support;

  const rosidl_message_type_support_t * type_support_introspection =
    access_path_[n].type_support_introspection;
  if (type_support_introspection->typesupport_identifier ==
    rosidl_typesupport_introspection_c__identifier)
  {
    is_c_type_support = true;
    ret = get_msg_data_address<rosidl_typesupport_introspection_c__MessageMembers>(
      type_support_introspection->data, access_path_[n], data);

  } else {
    is_c_type_support = false;
    ret = get_msg_data_address<rosidl_typesupport_introspection_cpp::MessageMembers>(
      type_support_introspection->data, access_path_[n], data);
  }

  if (ret) {
    if (last_step) {
      ret = set_member(data, is_c_type_support);

      has_value_ = true;
      value_has_changed();

      // Inform parent predicates
      for (FilterPredicate * parent : parents_) {
        parent->value_has_changed();
      }
    } else {
      ret = set_value(data, n + 1);
    }
  }

  return ret;
}

bool FilterField::set_member(
  const void * data,
  bool is_c_type_support)
{
  bool ret = true;

  switch (type_id_) {
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT:
      float_value = *reinterpret_cast<const float *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE:
      float_value = *reinterpret_cast<const double *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_LONG_DOUBLE:
      float_value = *reinterpret_cast<const long double *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
      char_value = *reinterpret_cast<const char *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOLEAN:
      boolean_value = *reinterpret_cast<const bool *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
      unsigned_integer_value = *reinterpret_cast<const uint8_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
      signed_integer_value = *reinterpret_cast<const int8_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
      unsigned_integer_value = *reinterpret_cast<const uint16_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
      signed_integer_value = *reinterpret_cast<const int16_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
      unsigned_integer_value = *reinterpret_cast<const uint32_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
      signed_integer_value = *reinterpret_cast<const int32_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
      unsigned_integer_value = *reinterpret_cast<const uint64_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
      signed_integer_value = *reinterpret_cast<const int64_t *>(data);
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:

      if (is_c_type_support) {
        auto c_string = reinterpret_cast<const rosidl_runtime_c__String *>(data);
#ifdef WIN32
        strncpy_s(
          string_value, sizeof(string_value), c_string->data, sizeof(string_value) - 1);
#else
        strncpy(string_value, c_string->data, sizeof(string_value) - 1);
#endif  // ifdef WIN32
      } else {
        auto string = reinterpret_cast<const std::string *>(data);
#ifdef WIN32
        strncpy_s(
          string_value, sizeof(string_value), string->c_str(), sizeof(string_value) - 1);
#else
        strncpy(string_value, string->c_str(), sizeof(string_value) - 1);
#endif  // ifdef WIN32
      }
      string_value[sizeof(string_value) - 1] = 0;
      break;

    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR:
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
    default:
      ret = false;
      break;
  }

  return ret;
}

}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback
