// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file IContentFilter.hpp
 */

#ifndef ICONTENTFILTER_HPP_
#define ICONTENTFILTER_HPP_


namespace rcl_content_filter_fallback
{

/**
 * The interface that content filter objects should implement.
 */
struct IContentFilter
{
  /**
   * Evaluate if a de-serialized payload should be accepted by certain reader.
   *
   * @param [in]  payload      The de-serialized payload of the sample being evaluated.
   *
   * @return whether the sample should be accepted.
   */
  virtual bool evaluate(
    const void * payload) const = 0;
};

}  // namespace rcl_content_filter_fallback

#endif  // ICONTENTFILTER_HPP_
