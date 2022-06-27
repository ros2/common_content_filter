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
 * @file FilterField.hpp
 */

#ifndef FILTERFIELD_HPP_
#define FILTERFIELD_HPP_

#include <rosidl_runtime_c/message_type_support_struct.h>

#include <cassert>
#include <unordered_set>
#include <vector>

#include "FilterPredicate.hpp"
#include "FilterValue.hpp"

namespace common_content_filter
{
namespace SQLFilter
{

/**
 * A FilterValue for fieldname-based expression values.
 */
class FilterField final : public FilterValue
{
public:
  /**
   * An element on the access path to the final field.
   */
  struct FieldAccessor final
  {
    /// Index of the member to access
    size_t member_index;

    /// Element index for array / sequence members
    size_t array_index;

    /// Type support introspection information for current field
    const rosidl_message_type_support_t * type_support_introspection;
  };

  /**
   * Construct a FilterField.
   *
   * @param[in]  type_id       TypeIdentifier representing the primitive data type of the fieldname.
   * @param[in]  access_path   Access path to the field.
   * @param[in]  data_kind     Kind of data the field represents.
   */
  FilterField(
    uint8_t type_id,
    const std::vector<FieldAccessor> & access_path,
    ValueKind data_kind)
  : FilterValue(data_kind)
    , access_path_(access_path)
    , type_id_(type_id)
  {
  }

  virtual ~FilterField() = default;

  /**
   * This method is used by a FilterPredicate to check if this FilterField can be used.
   *
   * @return whether this FilterField has a value that can be used on a predicate.
   */
  inline bool has_value() const noexcept final
  {
    return has_value_;
  }

  /**
   * Instruct this value to reset.
   */
  inline void reset() noexcept final
  {
    has_value_ = false;
  }


  /**
   * Perform the deserialization of the field represented by this FilterField.
   * Will notify the predicates where this FilterField is being used.
   *
   * @param[in]  data  The dynamic representation of the payload being filtered.
   *
   * @return Whether the deserialization process succeeded.
   *
   * @post Method @c has_value returns true.
   */
  inline bool set_value(
    const void * data_value)
  {
    return set_value(data_value, static_cast<size_t>(0));
  }

  /**
   * Perform the deserialization of a specific step of the access path.
   *
   * @param[in]  data  The dynamic representation of the step being processed.
   * @param[in]  n     The step on the access path being processed.
   *
   * @return Whether the deserialization process succeeded.
   *
   * @post Method @c has_value returns true.
   */
  bool set_value(
    const void * data_value,
    size_t n);

protected:
  inline void add_parent(
    FilterPredicate * parent) final
  {
    assert(nullptr != parent);
    parents_.emplace(parent);
  }

private:
  template<typename MembersType>
  bool
  get_msg_data_address(
    const void * untype_members,
    FieldAccessor & accessor,
    const void * & data);

  bool set_member(
    const void * data_value,
    bool is_c_type_support);

  bool has_value_ = false;
  std::vector<FieldAccessor> access_path_;
  uint8_t type_id_ = 0;
  std::unordered_set<FilterPredicate *> parents_;
};

}  // namespace SQLFilter
}  // namespace common_content_filter

#endif  // FILTERFIELD_HPP_
