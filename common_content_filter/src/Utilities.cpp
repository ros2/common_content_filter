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

#include "Utilities.hpp"

#include <rmw/error_handling.h>
#include <rosidl_typesupport_introspection_c/identifier.h>
#include <rosidl_typesupport_introspection_c/message_introspection.h>
#include <rosidl_typesupport_introspection_cpp/identifier.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>

#include "Log.hpp"

const rosidl_message_type_support_t *
get_type_support_introspection(
  const rosidl_message_type_support_t * type_support)
{
  const rosidl_message_type_support_t * type_support_introspection =
    get_message_typesupport_handle(
    type_support, rosidl_typesupport_introspection_c__identifier);
  if (nullptr == type_support_introspection) {
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    rcutils_reset_error();

    type_support_introspection =
      get_message_typesupport_handle(
      type_support,
      rosidl_typesupport_introspection_cpp::typesupport_identifier);
    if (nullptr == type_support_introspection) {
      rcutils_error_string_t error_string = rcutils_get_error_string();
      rcutils_reset_error();
      logError(
        SQLFILTER,
        "Type support not from this implementation. Got:\n"
          << "    " << prev_error_string.str << "\n"
          << "    " << error_string.str << "\n"
          << "while fetching it");
      return nullptr;
    }
  }

  return type_support_introspection;
}
