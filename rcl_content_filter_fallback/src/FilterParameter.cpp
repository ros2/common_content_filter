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
 * @file FilterParameter.cpp
 */

#include "FilterParameter.hpp"

#include "FilterExpressionParser.hpp"
#include "Log.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{

bool FilterParameter::set_value(
  const char * parameter)
{
  auto node = parser::parse_literal_value(parameter);

  if (!node) {
    logError(SQLFILTER, "PARSE ERROR: parser::parse_literal_value");

    return false;
  }

  copy_from(*node->left().value, false);
  value_has_changed();

  return true;
}

}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback
