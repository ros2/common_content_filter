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
 * @file FilterValue.hpp
 */

#ifndef FILTERVALUE_HPP_
#define FILTERVALUE_HPP_

#include <memory>
#include <regex>

namespace common_content_filter
{
namespace SQLFilter
{

class FilterPredicate;

/**
 * Represents a value (either constant, parameter or fieldname) on a filter expression.
 */
class FilterValue
{
public:
  // FilterPredicate needs to call protected method add_parent
  friend class FilterPredicate;

  /**
   * The high-level kind of a FilterValue.
   * Please note that the constants here should follow the promotion order.
   */
  enum class ValueKind
  {
    BOOLEAN,                ///< Value is a bool
    ENUM,                   ///< Value is an int32_t with the value of an enumeration
    SIGNED_INTEGER,         ///< Value is a int16_t, int32_t, or int64_t
    UNSIGNED_INTEGER,       ///< Value is a uint8_t, uint16_t, uint32_t, or uint64_t
    FLOAT_CONST,            ///< Value is a long double constant
    FLOAT_FIELD,            ///< Value is a float field
    DOUBLE_FIELD,           ///< Value is a double field
    LONG_DOUBLE_FIELD,      ///< Value is a long double field
    CHAR,                   ///< Value is a char
    STRING                  ///< Value is a string
  };

  /// The kind of value held by this FilterValue
  ValueKind kind;

  union {
    bool boolean_value;                                ///< Value when kind == BOOL
    char char_value;                                   ///< Value when kind == CHAR
    int64_t signed_integer_value;                      ///< Value when kind == SIGNED_INTEGER / ENUM
    uint64_t unsigned_integer_value;                   ///< Value when kind == UNSIGNED_INTEGER
    long double float_value;                           ///< Value when kind == FLOAT
    char string_value[255];       ///< Value when kind == STRING
  };

  /**
   * Default constructor.
   * Constructs an empty string FilterValue
   */
  FilterValue() noexcept
  : kind(ValueKind::STRING)
    , string_value()
  {
  }

  /**
   * Explicit kind constructor.
   * Constructs a zero-valued, specific kind FilterValue.
   *
   * @param[in] kind  The kind with which to construct the FilterValue.
   */
  explicit FilterValue(
    ValueKind data_kind) noexcept
  : kind(data_kind)
    , string_value()
  {
  }

  // *INDENT-OFF*
    FilterValue(const FilterValue&) = delete;
    FilterValue& operator=(const FilterValue&) = delete;
    FilterValue(FilterValue&&) = default;
    FilterValue& operator=(FilterValue&&) = default;
  // *INDENT-ON*

  virtual ~FilterValue() = default;

  /**
   * Copy the state of this object from another one.
   *
   * @param [in] other                    The FilterValue from where to copy the state.
   * @param [in] copy_regular_expression  Whether the regular expression state should be copied or not
   */
  void copy_from(
    const FilterValue & other,
    bool copy_regular_expression) noexcept;

  /**
   * This method is used by a FilterPredicate to check if this FilterValue can be used.
   * Constants and parameters will always have a value, but fieldname-based values can only be
   * used after deserialization.
   *
   * @return whether this FilterValue has a value that can be used on a predicate.
   */
  virtual bool has_value() const noexcept
  {
    return true;
  }

  /**
   * Instruct this value to reset.
   * Will only have effect on fieldname-based values.
   */
  virtual void reset() noexcept
  {
  }

  /**
   * Mark that this value should be handled as a regular expression.
   *
   * @param [in] is_like_operand  Whether this value is used on a LIKE or MATCH operation.
   */
  void as_regular_expression(
    bool is_like_operand);

  /**
   * @name Comparison operations
   * Methods implementing the comparison operators of binary predicates.
   * Should only be called against a FilterValue of a compatible kind,
   * according to the type promotion restrictions.
   */
  ///@{
  inline bool operator==(
    const FilterValue & other) const noexcept
  {
    return compare(*this, other) == 0;
  }

  inline bool operator!=(
    const FilterValue & other) const noexcept
  {
    return compare(*this, other) != 0;
  }

  inline bool operator<(
    const FilterValue & other) const noexcept
  {
    return compare(*this, other) < 0;
  }

  inline bool operator<=(
    const FilterValue & other) const noexcept
  {
    return compare(*this, other) <= 0;
  }

  inline bool operator>(
    const FilterValue & other) const noexcept
  {
    return compare(*this, other) > 0;
  }

  inline bool operator>=(
    const FilterValue & other) const noexcept
  {
    return compare(*this, other) >= 0;
  }

  bool is_like(
    const FilterValue & other) const noexcept;
  ///@}

protected:
  /**
   * Called when this FilterValue is used on a FilterPredicate.
   *
   * @param parent [in]  The FilterPredicate referencing this FilterValue.
   */
  virtual void add_parent(
    FilterPredicate * parent)
  {
    static_cast<void>(parent);
  }

  /**
   * Called when the value of this FilterValue has changed.
   * Will regenerate the regular expression object if as_regular_expression was called.
   */
  void value_has_changed();

private:
  enum class RegExpKind
  {
    NONE, LIKE, MATCH
  };

  RegExpKind regular_expr_kind_ = RegExpKind::NONE;
  std::unique_ptr<std::regex> regular_expr_;

  static int compare(
    const FilterValue & lhs,
    const FilterValue & rhs) noexcept;
};

}  // namespace SQLFilter
}  // namespace common_content_filter

#endif  // FILTERVALUE_HPP_
