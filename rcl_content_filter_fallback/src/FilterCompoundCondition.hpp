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
 * @file FilterCompoundCondition.hpp
 */

#ifndef FILTERCOMPOUNDCONDITION_HPP_
#define FILTERCOMPOUNDCONDITION_HPP_

#include <memory>

#include "FilterCondition.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{

/**
 * A FilterCondition that performs a logical operation over one or two FilterCondition objects.
 */
class FilterCompoundCondition final : public FilterCondition
{
public:
  /**
   * Possible kinds of logical operations
   */
  enum class OperationKind : uint8_t
  {
    NOT,      ///< NOT left
    AND,      ///< left AND right
    OR        ///< left OR right
  };

  /**
   * Construct a FilterCompoundCondition.
   *
   * @param[in]  op     Operation to perform.
   * @param[in]  left   Left operand.
   * @param[in]  right  Right operand.
   */
  FilterCompoundCondition(
    OperationKind op,
    std::unique_ptr<FilterCondition> && left,
    std::unique_ptr<FilterCondition> && right);

  virtual ~FilterCompoundCondition() = default;

protected:
  void propagate_reset() noexcept final;

  void child_has_changed(
    const FilterCondition & child) noexcept final;

private:
  OperationKind op_;
  std::unique_ptr<FilterCondition> left_;
  std::unique_ptr<FilterCondition> right_;
  uint8_t num_children_decided_ = 0;
};

}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback

#endif  // FILTERCOMPOUNDCONDITION_HPP_
