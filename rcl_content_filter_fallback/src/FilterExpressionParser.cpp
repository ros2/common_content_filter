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
 * @file FilterExpressionParser.cpp
 */

#include "FilterExpressionParser.hpp"

// header files needed by identifiers.hpp
#include <rmw/error_handling.h>
#include <rosidl_typesupport_introspection_c/identifier.h>
#include <rosidl_typesupport_introspection_c/message_introspection.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <rosidl_typesupport_introspection_cpp/identifier.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

#include "FilterGrammar.hpp"
#include "FilterParseNode.hpp"
#include "FilterValue.hpp"
#include "FilterField.hpp"
#include "Log.hpp"
#include "Utilities.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{
namespace parser
{

using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

#include "FilterExpressionParserImpl/identifiers.hpp"
#include "FilterExpressionParserImpl/literal_values.hpp"
#include "FilterExpressionParserImpl/parameters.hpp"
#include "FilterExpressionParserImpl/rearrange.hpp"

// select which rules in the grammar will produce parse tree nodes:
template<typename Rule>
using selector = parse_tree::selector<
  Rule,
  literal_value_processor::on<
    true_value,
    false_value,
    hex_value,
    integer_value,
    float_value,
    char_value,
    string_value>,
  parameter_processor::on<
    parameter_value>,
  parse_tree::store_content::on<
    string_content,
    integer,
    index_part,
    identifier>,
  parse_tree::remove_content::on<
    eq_op,
    gt_op,
    ge_op,
    lt_op,
    le_op,
    ne_op,
    like_op,
    match_op,
    and_op,
    or_op,
    not_op,
    dot_op,
    between_op,
    not_between_op>,
  rearrange::on<
    boolean_value,
    ComparisonPredicate,
    BetweenPredicate,
    Range,
    Condition,
    ConditionList>,
  identifier_processor::on<
    fieldname_part,
    fieldname>
>;

std::unique_ptr<ParseNode> parse_filter_expression(
  const char * expression,
  const rosidl_message_type_support_t * type_support)
{
  memory_input<> in(expression, "");
  try {
    CurrentIdentifierState identifier_state {type_support, nullptr, 0, {}};
    return parse_tree::parse<FilterExpressionGrammar, ParseNode, selector>(in, identifier_state);
  } catch (const parse_error & e) {
    const auto p = e.positions.front();
    logError(
      SQLFILTER, "PARSE ERROR: " << e.what() << std::endl
                                 << in.line_at(p) << std::endl
                                 << std::string(p.byte_in_line, ' ') << '^');
  } catch (const std::exception & e) {
    logError(SQLFILTER, "ERROR '" << e.what() << "' while parsing " << expression);
  }

  return nullptr;
}

std::unique_ptr<ParseNode> parse_literal_value(
  const char * expression)
{
  memory_input<> in(expression, "");
  try {
    CurrentIdentifierState identifier_state{nullptr, nullptr, 0, {}};
    return parse_tree::parse<LiteralGrammar, ParseNode, selector>(in, identifier_state);
  } catch (const parse_error & e) {
    const auto p = e.positions.front();
    logError(
      SQLFILTER, "PARSE ERROR: " << e.what() << std::endl
                                 << in.line_at(p) << std::endl
                                 << std::string(p.byte_in_line, ' ') << '^');
  } catch (const std::exception & e) {
    logError(SQLFILTER, "ERROR '" << e.what() << "' while parsing " << expression);
  }

  return nullptr;
}

}  // namespace parser
}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback
