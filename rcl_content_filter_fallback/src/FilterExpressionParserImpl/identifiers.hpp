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
 * @file identifiers.hpp
 *
 * Note: this is an implementation file, designed to be included inside the
 * FilterExpressionParser.hpp file of the parent folder.
 */

struct CurrentIdentifierState
{
  const rosidl_message_type_support_t * type_support;

  const rosidl_message_type_support_t * current_type_support;

  uint8_t current_type;

  std::vector<FilterField::FieldAccessor> access_path;
};

struct identifier_processor
  : parse_tree::apply<identifier_processor>
{
  template<typename MembersType>
  static void
  add_access_path(
    CurrentIdentifierState & identifier_state,
    std::unique_ptr<ParseNode> & n,
    const rosidl_message_type_support_t * type_support_introspection)
  {
    const MembersType * members =
      static_cast<const MembersType *>(type_support_introspection->data);
    if (!members) {
      throw std::runtime_error("The data in the type support introspection is invalid.");
    }

    size_t member_index = 0;
    size_t array_index = std::numeric_limits<size_t>::max();

    const ParseNode & name_node = n->left();
    std::string parse_node_name = name_node.content();

    for (uint32_t i = 0; i < members->member_count_; ++i) {
      const auto member = members->members_ + i;

      if (member->name_ == parse_node_name) {
        member_index = i;
        bool has_index = n->children.size() > 1;
        if (member->is_array_) {
          if (!has_index) {
            throw parse_error("field should have an index (i.e. [n])", n->left().end());
          }

          array_index = static_cast<size_t>(std::stoul(n->right().left().content()));

          if (member->array_size_ && !member->is_upper_bound_) {
            if (member->array_size_ <= array_index) {
              throw parse_error("index is greater than maximum size", n->right().end());
            }
          }
        } else {
          if (has_index) {
            throw parse_error("field is not an array or sequence", n->right().begin());
          }
        }

        identifier_state.current_type = member->type_id_;

        if (member->type_id_ == ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
          identifier_state.current_type_support = member->members_;
        }

        identifier_state.access_path.emplace_back(
          FilterField::FieldAccessor{member_index, array_index, type_support_introspection});

        return;
      }
    }
    throw parse_error("field not found", name_node.begin());
  }

  static void add_member_access(
    std::unique_ptr<ParseNode> & n,
    CurrentIdentifierState & identifier_state,
    const rosidl_message_type_support_t * type_support)
  {
    const rosidl_message_type_support_t * type_support_introspection =
      get_type_support_introspection(type_support);
    if (!type_support_introspection) {
      throw std::runtime_error("failed to get type support introspection");
    }

    if (type_support_introspection->typesupport_identifier ==
      rosidl_typesupport_introspection_c__identifier)
    {
      add_access_path<rosidl_typesupport_introspection_c__MessageMembers>(
        identifier_state, n, type_support_introspection);
    } else {
      add_access_path<rosidl_typesupport_introspection_cpp::MessageMembers>(
        identifier_state, n, type_support_introspection);
    }
  }

  static FilterValue::ValueKind get_value_kind(
    uint8_t type_id,
    const position & pos)
  {
    switch (type_id) {
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOLEAN:
        return FilterValue::ValueKind::BOOLEAN;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
        return FilterValue::ValueKind::CHAR;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        return FilterValue::ValueKind::STRING;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        return FilterValue::ValueKind::SIGNED_INTEGER;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        return FilterValue::ValueKind::UNSIGNED_INTEGER;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT:
        return FilterValue::ValueKind::FLOAT_FIELD;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE:
        return FilterValue::ValueKind::DOUBLE_FIELD;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_LONG_DOUBLE:
        return FilterValue::ValueKind::LONG_DOUBLE_FIELD;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
        break;
    }

    throw parse_error("type is not primitive", pos);
  }

  template<typename ... States>
  static void transform(
    std::unique_ptr<ParseNode> & n,
    CurrentIdentifierState & state,
    States &&... /*st*/)
  {
    if (n->is<fieldname>()) {
      // Set data for fieldname node
      n->field_kind = get_value_kind(state.current_type, n->end());
      n->field_access_path = state.access_path;
      n->type_id = state.current_type;

      // Reset parser state
      state.access_path.clear();
      state.current_type = 0;
      state.current_type_support = nullptr;
    } else {
      if (0 == state.current_type) {
        add_member_access(n, state, state.type_support);
      } else {
        add_member_access(n, state, state.current_type_support);
      }
    }

    n->children.clear();
  }
};
