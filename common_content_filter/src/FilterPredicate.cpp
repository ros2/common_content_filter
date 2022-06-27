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
 * @file FilterPredicate.cpp
 */

#include "FilterPredicate.hpp"

#include <cassert>
#include <memory>

#include "FilterValue.hpp"

namespace common_content_filter
{
namespace SQLFilter
{

FilterPredicate::FilterPredicate(
  OperationKind op,
  const std::shared_ptr<FilterValue> & left,
  const std::shared_ptr<FilterValue> & right)
: op_(op)
  , left_(left)
  , right_(right)
{
  assert(left_);
  assert(right_);

  left_->add_parent(this);
  right_->add_parent(this);

  if (OperationKind::LIKE == op_) {
    right_->as_regular_expression(true);
  } else if (OperationKind::MATCH == op_) {
    right_->as_regular_expression(false);
  }
}

void FilterPredicate::value_has_changed()
{
  if (left_->has_value() && right_->has_value()) {
    switch (op_) {
      case OperationKind::EQUAL:
        set_result(*left_ == *right_);
        break;

      case OperationKind::NOT_EQUAL:
        set_result(*left_ != *right_);
        break;

      case OperationKind::LESS_THAN:
        set_result(*left_ < *right_);
        break;

      case OperationKind::LESS_EQUAL:
        set_result(*left_ <= *right_);
        break;

      case OperationKind::GREATER_THAN:
        set_result(*left_ > *right_);
        break;

      case OperationKind::GREATER_EQUAL:
        set_result(*left_ >= *right_);
        break;

      case OperationKind::LIKE:
      case OperationKind::MATCH:
        set_result(left_->is_like(*right_));
        break;

      default:
        assert(false);
    }
  }
}

void FilterPredicate::propagate_reset() noexcept
{
  left_->reset();
  right_->reset();
}

}  // namespace SQLFilter
}  // namespace common_content_filter
