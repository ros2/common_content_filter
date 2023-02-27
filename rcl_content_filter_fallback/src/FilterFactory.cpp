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
 * @file FilterFactory.cpp
 */

#include "FilterFactory.hpp"

#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "IContentFilter.hpp"
#include "IContentFilterFactory.hpp"
#include "FilterCompoundCondition.hpp"
#include "FilterCondition.hpp"
#include "FilterConditionState.hpp"
#include "FilterEmptyExpression.hpp"
#include "FilterExpression.hpp"
#include "FilterExpressionParser.hpp"
#include "FilterField.hpp"
#include "FilterGrammar.hpp"
#include "FilterParseNode.hpp"
#include "FilterParameter.hpp"
#include "FilterPredicate.hpp"
#include "FilterValue.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{

static IContentFilterFactory::ReturnCode_t transform_enum(
  std::shared_ptr<FilterValue> & value,
  uint8_t type,
  const std::string & string_value)
{
  static_cast<void>(value);
  static_cast<void>(type);
  static_cast<void>(string_value);
  // TODO(iuhilnehc-ynos): when enum supported in the rosidl
  return IContentFilterFactory::ReturnCode_t::RETCODE_BAD_PARAMETER;
}

static IContentFilterFactory::ReturnCode_t transform_enums(
  std::shared_ptr<FilterValue> & left_value,
  uint8_t left_type,
  std::shared_ptr<FilterValue> & right_value,
  uint8_t right_type)
{
  if ((FilterValue::ValueKind::ENUM == left_value->kind) &&
    (FilterValue::ValueKind::STRING == right_value->kind))
  {
    return transform_enum(right_value, left_type, right_value->string_value);
  }

  if ((FilterValue::ValueKind::ENUM == right_value->kind) &&
    (FilterValue::ValueKind::STRING == left_value->kind))
  {
    return transform_enum(left_value, right_type, left_value->string_value);
  }

  return IContentFilterFactory::ReturnCode_t::RETCODE_OK;
}

static bool check_value_compatibility(
  FilterValue::ValueKind left,
  FilterValue::ValueKind right,
  bool ignore_enum)
{
  if (!ignore_enum && FilterValue::ValueKind::ENUM == right) {
    return FilterValue::ValueKind::ENUM == left ||
           FilterValue::ValueKind::SIGNED_INTEGER == left ||
           FilterValue::ValueKind::UNSIGNED_INTEGER == left ||
           FilterValue::ValueKind::STRING == left;
  }

  switch (left) {
    case FilterValue::ValueKind::BOOLEAN:
      return FilterValue::ValueKind::BOOLEAN == right ||
             FilterValue::ValueKind::SIGNED_INTEGER == right ||
             FilterValue::ValueKind::UNSIGNED_INTEGER == right;

    case FilterValue::ValueKind::SIGNED_INTEGER:
    case FilterValue::ValueKind::UNSIGNED_INTEGER:
      return FilterValue::ValueKind::SIGNED_INTEGER == right ||
             FilterValue::ValueKind::UNSIGNED_INTEGER == right ||
             FilterValue::ValueKind::BOOLEAN == right ||
             FilterValue::ValueKind::FLOAT_CONST == right ||
             FilterValue::ValueKind::FLOAT_FIELD == right ||
             FilterValue::ValueKind::DOUBLE_FIELD == right ||
             FilterValue::ValueKind::LONG_DOUBLE_FIELD == right;

    case FilterValue::ValueKind::CHAR:
    case FilterValue::ValueKind::STRING:
      return FilterValue::ValueKind::CHAR == right ||
             FilterValue::ValueKind::STRING == right;

    case FilterValue::ValueKind::FLOAT_CONST:
    case FilterValue::ValueKind::FLOAT_FIELD:
    case FilterValue::ValueKind::DOUBLE_FIELD:
    case FilterValue::ValueKind::LONG_DOUBLE_FIELD:
      return FilterValue::ValueKind::FLOAT_CONST == right ||
             FilterValue::ValueKind::FLOAT_FIELD == right ||
             FilterValue::ValueKind::DOUBLE_FIELD == right ||
             FilterValue::ValueKind::LONG_DOUBLE_FIELD == right ||
             FilterValue::ValueKind::SIGNED_INTEGER == right ||
             FilterValue::ValueKind::UNSIGNED_INTEGER == right;

    case FilterValue::ValueKind::ENUM:
      if (!ignore_enum) {
        return FilterValue::ValueKind::ENUM == right ||
               FilterValue::ValueKind::SIGNED_INTEGER == right ||
               FilterValue::ValueKind::UNSIGNED_INTEGER == right ||
               FilterValue::ValueKind::STRING == right;
      }
  }

  return false;
}

static FilterPredicate::OperationKind get_predicate_op(
  const parser::ParseNode & node)
{
  FilterPredicate::OperationKind ret_val = FilterPredicate::OperationKind::EQUAL;
  if (node.is<eq_op>()) {
    ret_val = FilterPredicate::OperationKind::EQUAL;
  } else if (node.is<ne_op>()) {
    ret_val = FilterPredicate::OperationKind::NOT_EQUAL;
  } else if (node.is<lt_op>()) {
    ret_val = FilterPredicate::OperationKind::LESS_THAN;
  } else if (node.is<le_op>()) {
    ret_val = FilterPredicate::OperationKind::LESS_EQUAL;
  } else if (node.is<gt_op>()) {
    ret_val = FilterPredicate::OperationKind::GREATER_THAN;
  } else if (node.is<ge_op>()) {
    ret_val = FilterPredicate::OperationKind::GREATER_EQUAL;
  } else if (node.is<like_op>()) {
    ret_val = FilterPredicate::OperationKind::LIKE;
  } else if (node.is<match_op>()) {
    ret_val = FilterPredicate::OperationKind::MATCH;
  } else {
    assert(false);
  }

  return ret_val;
}

struct ExpressionParsingState
{
  const void * type_object;
  const IContentFilterFactory::ParameterSeq & filter_parameters;
  FilterExpression * filter;
};

template<>
IContentFilterFactory::ReturnCode_t FilterFactory::convert_tree<FilterCondition>(
  ExpressionParsingState & state,
  std::unique_ptr<FilterCondition> & condition,
  const parser::ParseNode & node);


template<>
IContentFilterFactory::ReturnCode_t FilterFactory::convert_tree<FilterValue>(
  ExpressionParsingState & state,
  std::shared_ptr<FilterValue> & value,
  const parser::ParseNode & node)
{
  if (node.value) {
    value = std::make_shared<FilterValue>();
    value->copy_from(*node.value.get(), true);
  } else if (0 != node.type_id) {
    std::string field_name = node.content();
    auto it = state.filter->fields.find(field_name);
    if (it == state.filter->fields.end()) {
      value = state.filter->fields[field_name] =
        std::make_shared<FilterField>(node.type_id, node.field_access_path, node.field_kind);
    } else {
      value = it->second;
    }
  } else {
    // Check parameter index
    if (node.parameter_index >= state.filter_parameters.size()) {
      return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    if (state.filter->parameters[node.parameter_index]) {
      value = state.filter->parameters[node.parameter_index];
    } else {
      auto param_value = std::make_shared<FilterParameter>();
      if (!param_value->set_value(state.filter_parameters[node.parameter_index].c_str())) {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
      }
      value = state.filter->parameters[node.parameter_index] = param_value;
    }
  }

  return ReturnCode_t::RETCODE_OK;
}

template<>
IContentFilterFactory::ReturnCode_t FilterFactory::convert_tree<FilterPredicate>(
  ExpressionParsingState & state,
  std::unique_ptr<FilterCondition> & condition,
  const parser::ParseNode & node)
{
  std::shared_ptr<FilterValue> left;
  std::shared_ptr<FilterValue> right;
  ReturnCode_t ret = convert_tree<FilterValue>(state, left, node.left());
  if (ReturnCode_t::RETCODE_OK == ret) {
    ret = convert_tree<FilterValue>(state, right, node.right());
    if (ReturnCode_t::RETCODE_OK == ret) {
      bool ignore_enum = false;
      if (node.is<like_op>() || node.is<match_op>()) {
        // At least one fieldname should be a string
        if (!((node.left().is<fieldname>() && (FilterValue::ValueKind::STRING == left->kind)) ||
          (node.right().is<fieldname>() && (FilterValue::ValueKind::STRING == right->kind))))
        {
          return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        ignore_enum = true;
      }

      if ((FilterValue::ValueKind::ENUM == left->kind) &&
        (FilterValue::ValueKind::ENUM == right->kind))
      {
        if (node.left().type_id != node.right().type_id) {
          return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
      } else if (!check_value_compatibility(left->kind, right->kind, ignore_enum)) {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
      }

      ret = transform_enums(left, node.left().type_id, right, node.right().type_id);
      if (ReturnCode_t::RETCODE_OK == ret) {
        condition.reset(new FilterPredicate(get_predicate_op(node), left, right));
      }
    }
  }

  return ret;
}

template<>
IContentFilterFactory::ReturnCode_t FilterFactory::convert_tree<between_op>(
  ExpressionParsingState & state,
  std::unique_ptr<FilterCondition> & condition,
  const parser::ParseNode & node)
{
  /* The nodes here will be in the following situation:
   *
   *          between_op
   *          /         \
   * fieldname           and_op
   *                    /      \
   *                 op1        op2
   */

  std::shared_ptr<FilterValue> field;
  ReturnCode_t ret = convert_tree<FilterValue>(state, field, node.left());
  if (ReturnCode_t::RETCODE_OK == ret) {
    const parser::ParseNode & and_node = node.right();
    assert(and_node.is<and_op>());

    std::shared_ptr<FilterValue> op1;
    std::shared_ptr<FilterValue> op2;

    ret = convert_tree<FilterValue>(state, op1, and_node.left());
    if (ReturnCode_t::RETCODE_OK == ret) {
      ret = convert_tree<FilterValue>(state, op2, and_node.right());
    }

    if (ReturnCode_t::RETCODE_OK == ret) {
      if (!check_value_compatibility(field->kind, op1->kind, false) ||
        !check_value_compatibility(field->kind, op2->kind, false) ||
        !check_value_compatibility(op1->kind, op2->kind, false))
      {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
      }

      ret = transform_enums(field, node.left().type_id, op1, and_node.left().type_id);
      if (ReturnCode_t::RETCODE_OK == ret) {
        ret = transform_enums(field, node.left().type_id, op2, and_node.right().type_id);
      }
    }

    if (ReturnCode_t::RETCODE_OK == ret) {
      FilterPredicate::OperationKind binary_op = node.is<between_op>() ?
        FilterPredicate::OperationKind::LESS_EQUAL :
        FilterPredicate::OperationKind::GREATER_THAN;
      FilterCompoundCondition::OperationKind logical_op = node.is<between_op>() ?
        FilterCompoundCondition::OperationKind::AND :
        FilterCompoundCondition::OperationKind::OR;

      std::unique_ptr<FilterCondition> left_cond(new FilterPredicate(binary_op, op1, field));
      std::unique_ptr<FilterCondition> right_cond(new FilterPredicate(binary_op, field, op2));
      auto cond =
        new FilterCompoundCondition(
        logical_op, std::move(left_cond), std::move(
          right_cond));
      condition.reset(cond);
    }
  }

  return ret;
}

template<>
IContentFilterFactory::ReturnCode_t FilterFactory::convert_tree<FilterCompoundCondition>(
  ExpressionParsingState & state,
  std::unique_ptr<FilterCondition> & condition,
  const parser::ParseNode & node)
{
  ReturnCode_t ret = ReturnCode_t::RETCODE_UNSUPPORTED;
  FilterCompoundCondition::OperationKind op = FilterCompoundCondition::OperationKind::NOT;
  std::unique_ptr<FilterCondition> left;
  std::unique_ptr<FilterCondition> right;

  if (node.is<not_op>()) {
    op = FilterCompoundCondition::OperationKind::NOT;
    ret = convert_tree<FilterCondition>(state, left, node.left());
  } else if (node.is<and_op>()) {
    op = FilterCompoundCondition::OperationKind::AND;
    ret = convert_tree<FilterCondition>(state, left, node.left());
    if (ReturnCode_t::RETCODE_OK == ret) {
      ret = convert_tree<FilterCondition>(state, right, node.right());
    }
  } else if (node.is<or_op>()) {
    op = FilterCompoundCondition::OperationKind::OR;
    ret = convert_tree<FilterCondition>(state, left, node.left());
    if (ReturnCode_t::RETCODE_OK == ret) {
      ret = convert_tree<FilterCondition>(state, right, node.right());
    }
  } else {
    assert(false);
  }

  if (ReturnCode_t::RETCODE_OK == ret) {
    condition.reset(new FilterCompoundCondition(op, std::move(left), std::move(right)));
  }

  return ret;
}

template<>
IContentFilterFactory::ReturnCode_t FilterFactory::convert_tree<FilterCondition>(
  ExpressionParsingState & state,
  std::unique_ptr<FilterCondition> & condition,
  const parser::ParseNode & node)
{
  if (node.is<and_op>() || node.is<or_op>() || node.is<not_op>()) {
    return convert_tree<FilterCompoundCondition>(state, condition, node);
  } else if (node.is<between_op>() || node.is<not_between_op>()) {
    return convert_tree<between_op>(state, condition, node);
  }

  return convert_tree<FilterPredicate>(state, condition, node);
}

FilterFactory::~FilterFactory()
{
  auto & pool = expression_pool_.collection();
  for (FilterExpression * item : pool) {
    delete item;
  }
  pool.clear();
}

IContentFilterFactory::ReturnCode_t FilterFactory::create_content_filter(
  const rosidl_message_type_support_t * type_support,
  const char * filter_expression,
  const IContentFilterFactory::ParameterSeq & filter_parameters,
  IContentFilter * & filter_instance)
{
  ReturnCode_t ret = ReturnCode_t::RETCODE_UNSUPPORTED;

  if (nullptr == filter_expression) {
    if (nullptr == filter_instance) {
      ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
    } else {
      ret = ReturnCode_t::RETCODE_OK;
      if (&empty_expression_ != filter_instance) {
        auto expr = static_cast<FilterExpression *>(filter_instance);
        auto n_params = expr->parameters.size();
        if (filter_parameters.size() < n_params) {
          ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
        } else {
          std::vector<FilterValue> old_values(n_params);
          size_t n = n_params;
          while ((n > 0) && (ReturnCode_t::RETCODE_OK == ret)) {
            --n;
            if (expr->parameters[n]) {
              old_values[n].copy_from(*(expr->parameters[n]), true);
              if (!expr->parameters[n]->set_value(filter_parameters[n].c_str())) {
                ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
              }
            }
          }

          if (ReturnCode_t::RETCODE_OK != ret) {
            while (n < n_params) {
              expr->parameters[n]->copy_from(old_values[n], true);
              ++n;
            }
          }
        }
      }
    }
  } else if (std::strlen(filter_expression) == 0) {
    delete_content_filter(filter_instance);
    filter_instance = nullptr;
    ret = ReturnCode_t::RETCODE_OK;
  } else {
    auto node = parser::parse_filter_expression(filter_expression, type_support);
    if (node) {
      FilterExpression * expr = get_expression();
      size_t n_params = filter_parameters.size();
      expr->parameters.reserve(n_params);
      while (expr->parameters.size() < n_params) {
        expr->parameters.emplace_back();
      }
      ExpressionParsingState state{nullptr, filter_parameters, expr};
      ret = convert_tree<FilterCondition>(state, expr->root, *(node->children[0]));
      if (ReturnCode_t::RETCODE_OK == ret) {
        delete_content_filter(filter_instance);
        filter_instance = expr;
      } else {
        delete_content_filter(expr);
      }
    } else {
      ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
  }

  return ret;
}

IContentFilterFactory::ReturnCode_t FilterFactory::delete_content_filter(
  IContentFilter * filter_instance)
{
  if (nullptr == filter_instance) {
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
  }

  if (&empty_expression_ != filter_instance) {
    auto expr = static_cast<FilterExpression *>(filter_instance);
    expr->clear();
    expression_pool_.put(expr);
  }
  return ReturnCode_t::RETCODE_OK;
}

}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback
