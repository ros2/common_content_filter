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
 * @file FilterPredicate.hpp
 */

#ifndef FILTERPREDICATE_HPP_
#define FILTERPREDICATE_HPP_

#include <memory>

#include "FilterCondition.hpp"
#include "FilterValue.hpp"

namespace common_content_filter
{
namespace SQLFilter
{

/**
 * A FilterCondition for binary predicates (i.e. <op1> <operator> <op2>).
 */
class FilterPredicate final : public FilterCondition
{
public:
  /**
   * Possible kinds of binary operations
   */
  enum class OperationKind : uint8_t
  {
    EQUAL,              ///< left = right
    NOT_EQUAL,          ///< left <> right
    LESS_THAN,          ///< left < right
    LESS_EQUAL,         ///< left <= right
    GREATER_THAN,       ///< left > right
    GREATER_EQUAL,      ///< left >= right
    LIKE,               ///< left LIKE right
    MATCH               ///< left MATCH right
  };

  /**
   * Construct a FilterPredicate.
   *
   * @param[in]  op     Operation to perform.
   * @param[in]  left   Left operand.
   * @param[in]  right  Right operand.
   */
  FilterPredicate(
    OperationKind op,
    const std::shared_ptr<FilterValue> & left,
    const std::shared_ptr<FilterValue> & right);

  virtual ~FilterPredicate() = default;

  /**
   * Called when the value of an operand is changed.
   */
  void value_has_changed();

protected:
  void propagate_reset() noexcept final;

  void child_has_changed(
    const FilterCondition & child) noexcept final
  {
    static_cast<void>(child);
  }

private:
  OperationKind op_;
  std::shared_ptr<FilterValue> left_;
  std::shared_ptr<FilterValue> right_;
};

}  // namespace SQLFilter
}  // namespace common_content_filter

#endif  // FILTERPREDICATE_HPP_
