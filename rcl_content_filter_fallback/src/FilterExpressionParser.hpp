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
 * @file FilterExpressionParser.hpp
 */

#ifndef FILTEREXPRESSIONPARSER_HPP_
#define FILTEREXPRESSIONPARSER_HPP_

#include <rosidl_runtime_c/message_type_support_struct.h>
#include <memory>
#include "FilterParseNode.hpp"


namespace rcl_content_filter_fallback
{
namespace SQLFilter
{
namespace parser
{

/**
 * Performs parsing of a string containing a -SQL filter expression.
 *
 * @param[in]  expression   The string to parse.
 * @param[in]  type_support  The type support representing the type of the data being filtered.
 *
 * @return nullptr when there is a parsing error.
 * @return A pointer to the root node of the AST tree for the expression.
 */
std::unique_ptr<ParseNode> parse_filter_expression(
  const char * expression,
  const rosidl_message_type_support_t * type_support);

/**
 * Performs parsing of a string containing a literal value.
 * This method is used to perform parsing of parameter values.
 *
 * @param[in]  value  The string to parse.
 *
 * @return nullptr when there is a parsing error.
 * @return A simple tree consisting of a root node, with a single child that contains the generated FilterValue.
 */
std::unique_ptr<ParseNode> parse_literal_value(
  const char * value);

}  // namespace parser
}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback

#endif  // FILTEREXPRESSIONPARSER_HPP_
