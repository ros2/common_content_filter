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
 * @file FilterParseNode.hpp
 */

#ifndef FILTERPARSENODE_HPP_
#define FILTERPARSENODE_HPP_

#include <memory>
#include <vector>

#include <tao/pegtl/contrib/parse_tree.hpp>

#include "FilterField.hpp"
#include "FilterValue.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{
namespace parser
{

using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

struct ParseNode : parse_tree::basic_node<ParseNode>
{
  // When the node is a literal value, it will hold a pointer to it
  std::unique_ptr<FilterValue> value;

  // When the node is a fieldname, it will hold the access path to the field, the data kind,
  // and the type id
  std::vector<FilterField::FieldAccessor> field_access_path;
  FilterValue::ValueKind field_kind = FilterValue::ValueKind::STRING;
  // ros2 primitive type id
  uint8_t type_id = 0;

  // When the node is a parameter, it will hold the parameter index
  size_t parameter_index = 0;

  const ParseNode & left() const
  {
    return *(children[0]);
  }

  const ParseNode & right() const
  {
    return *(children[1]);
  }
};

}  // namespace parser
}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback

#endif  // FILTERPARSENODE_HPP_
