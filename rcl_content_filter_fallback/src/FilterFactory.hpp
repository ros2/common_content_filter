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
 * @file FilterFactory.hpp
 */

#ifndef FILTERFACTORY_HPP_
#define FILTERFACTORY_HPP_

#include "IContentFilterFactory.hpp"

#include "IContentFilter.hpp"

#include "FilterEmptyExpression.hpp"
#include "FilterExpression.hpp"
#include "ObjectPool.hpp"

namespace rcl_content_filter_fallback
{
namespace SQLFilter
{

/**
 * An IContentFilterFactory that processes -SQL filter expressions.
 */
class FilterFactory final : public IContentFilterFactory
{
public:
  ~FilterFactory();

  ReturnCode_t create_content_filter(
    const rosidl_message_type_support_t * type_support,
    const char * filter_expression,
    const ParameterSeq & filter_parameters,
    IContentFilter * & filter_instance) override;

  ReturnCode_t delete_content_filter(
    IContentFilter * filter_instance) override;

private:
  /**
   * Retrieve a FilterExpression from the pool.
   *
   * @return A pointer to an empty FilterExpression.
   */
  FilterExpression * get_expression()
  {
    return expression_pool_.get(
      []
      {
        return new FilterExpression();
      });
  }

  /**
   * Generic method to perform processing of an AST node resulting from the parsing of a -SQL filter expression.
   * Provides a generic mechanism for methods that perform post-processing of the generated AST tree, so they could
   * have access to the private fields of FilterFactory.
   *
   * @return return code indicating the conversion result.
   */
  template<typename _Parser, typename _ParserNode, typename _State, typename _Output>
  ReturnCode_t convert_tree(
    _State & state,
    _Output & parse_output,
    const _ParserNode & node);

  /// Empty expressions content filter
  FilterEmptyExpression empty_expression_;
  /// Pool of FilterExpression objects
  ObjectPool<FilterExpression *> expression_pool_;
};

}  // namespace SQLFilter
}  // namespace rcl_content_filter_fallback

#endif  // FILTERFACTORY_HPP_
