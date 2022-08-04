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
 * @file FilterCondition.hpp
 */

#ifndef FILTERCONDITION_HPP_
#define FILTERCONDITION_HPP_

#include "FilterConditionState.hpp"

namespace common_content_filter
{
namespace SQLFilter
{

/**
 * Base class for conditions on a filter expression.
 */
class FilterCondition
{
public:
  friend class FilterCompoundCondition;

  virtual ~FilterCondition() = default;

  /**
   * @return the current state of this condition.
   */
  inline FilterConditionState get_state() const noexcept
  {
    return state_;
  }

  /**
   * Instruct this condition to reset.
   * Will propagate the reset command down the expression tree.
   *
   * @post The state of this condition will be UNDECIDED.
   */
  inline void reset() noexcept
  {
    state_ = FilterConditionState::UNDECIDED;
    propagate_reset();
  }

protected:
  /**
   * Set a new state for this condition.
   * May propagate the change up the expression tree by calling
   * @ref child_has_changed on the parent of this condition.
   *
   * @param[in] state New state for this condition.
   *
   * @post The state of this condition will be @c state.
   */
  inline void set_state(
    FilterConditionState state) noexcept
  {
    if (state != state_) {
      state_ = state;
      if (nullptr != parent_) {
        parent_->child_has_changed(*this);
      }
    }
  }

  /**
   * Set the result of this condition.
   *
   * @param[in]  result   The result to be set.
   *
   * @post The state of this condition will not be UNDECIDED.
   */
  inline void set_result(
    bool result) noexcept
  {
    set_state(result ? FilterConditionState::RESULT_TRUE : FilterConditionState::RESULT_FALSE);
  }

  /**
   * Set a new parent for this condition.
   *
   * @param parent  New parent to set.
   */
  inline void set_parent(
    FilterCondition * parent) noexcept
  {
    parent_ = parent;
  }

  /**
   * Propagates the reset command down the expression tree.
   */
  virtual void propagate_reset() noexcept = 0;


  /**
   * A child condition will call this method whenever its state is changed.
   *
   * @param[in] child The child condition
   */
  virtual void child_has_changed(
    const FilterCondition & child) noexcept = 0;

private:
  FilterConditionState state_ = FilterConditionState::UNDECIDED;
  FilterCondition * parent_ = nullptr;
};

}  // namespace SQLFilter
}  // namespace common_content_filter

#endif  // FILTERCONDITION_HPP_
