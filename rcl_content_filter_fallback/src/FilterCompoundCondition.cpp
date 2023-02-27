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
 * @file FilterCompoundCondition.cpp
 */

#include "FilterCompoundCondition.hpp"

#include <cassert>
#include <memory>
#include <utility>

#include "FilterCondition.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{

FilterCompoundCondition::FilterCompoundCondition(
  OperationKind op,
  std::unique_ptr<FilterCondition> && left,
  std::unique_ptr<FilterCondition> && right)
: op_(op)
  , left_(std::move(left))
  , right_(std::move(right))
{
  assert(left_);
  assert(right_ || OperationKind::NOT == op_);

  left_->set_parent(this);
  if (right_) {
    right_->set_parent(this);
  }
}

void FilterCompoundCondition::propagate_reset() noexcept
{
  num_children_decided_ = 0;

  left_->reset();
  if (right_) {
    right_->reset();
  }
}

void FilterCompoundCondition::child_has_changed(
  const FilterCondition & child) noexcept
{
  FilterConditionState child_state = child.get_state();
  assert(FilterConditionState::UNDECIDED != child_state);

  ++num_children_decided_;

  if (FilterConditionState::UNDECIDED == get_state()) {
    switch (op_) {
      case OperationKind::NOT:
        set_result(FilterConditionState::RESULT_FALSE == child_state);
        break;

      case OperationKind::AND:
        if (FilterConditionState::RESULT_FALSE == child_state) {
          set_result(false);
        } else {
          if (2 == num_children_decided_) {
            set_result(true);
          }
        }
        break;

      case OperationKind::OR:
        if (FilterConditionState::RESULT_TRUE == child_state) {
          set_result(true);
        } else {
          if (2 == num_children_decided_) {
            set_result(false);
          }
        }
        break;

      default:
        assert(false);
    }
  }
}

}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback
