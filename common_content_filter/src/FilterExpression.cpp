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
 * @file FilterExpression.cpp
 */

#include "FilterExpression.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "FilterCondition.hpp"
#include "FilterField.hpp"
#include "FilterParameter.hpp"

namespace common_content_filter
{
namespace SQLFilter
{

bool FilterExpression::evaluate(
  const void * payload) const
{
  root->reset();
  for (auto it = fields.begin();
    it != fields.end() && FilterConditionState::UNDECIDED == root->get_state();
    ++it)
  {
    if (!it->second->set_value(payload)) {
      return false;
    }
  }

  return FilterConditionState::RESULT_TRUE == root->get_state();
}

void FilterExpression::clear()
{
  parameters.clear();
  fields.clear();
  root.reset();
}


}  // namespace SQLFilter
}  // namespace common_content_filter
