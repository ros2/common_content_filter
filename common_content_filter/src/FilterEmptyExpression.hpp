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
 * @file FilterEmptyExpression.hpp
 */

#ifndef FILTEREMPTYEXPRESSION_HPP_
#define FILTEREMPTYEXPRESSION_HPP_

#include "IContentFilter.hpp"

namespace common_content_filter
{
namespace SQLFilter
{

/**
 * An IContentFilter for empty expressions that always evaluates to true.
 */
class FilterEmptyExpression final : public IContentFilter
{
public:
  bool evaluate(
    const void * payload) const final
  {
    static_cast<void>(payload);

    return true;
  }
};

}  // namespace SQLFilter
}  // namespace common_content_filter

#endif  // FILTEREMPTYEXPRESSION_HPP_
