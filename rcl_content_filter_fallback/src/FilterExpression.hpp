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
 * @file FilterExpression.hpp
 */

#ifndef FILTEREXPRESSION_HPP_
#define FILTEREXPRESSION_HPP_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "IContentFilter.hpp"
#include "FilterCondition.hpp"
#include "FilterField.hpp"
#include "FilterParameter.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{

/**
 * An IContentFilter that evaluates -SQL filter expressions
 */
class FilterExpression final : public IContentFilter
{
public:
  bool evaluate(
    const void * payload) const final;

  /**
   * Clear the information held by this object.
   */
  void clear();

  /// The root condition of the expression tree.
  std::unique_ptr<FilterCondition> root;
  /// The fields referenced by this expression.
  std::map<std::string, std::shared_ptr<FilterField>> fields;
  /// The parameters referenced by this expression.
  std::vector<std::shared_ptr<FilterParameter>> parameters;
};

}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback

#endif  // FILTEREXPRESSION_HPP_
