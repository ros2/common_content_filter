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

#include <rmw/rmw.h>
#include <rmw/serialized_message.h>
#include <test_msgs/msg/basic_types.h>

#include <gtest/gtest.h>
#include <tuple>
#include <vector>

#include <rosidl_typesupport_cpp/message_type_support.hpp>
#include <test_content_filter_msgs/msg/complex.hpp>
#include <osrf_testing_tools_cpp/scope_exit.hpp>

#include "common_content_filter/api.h"

class TestAPIBase
{
protected:
  bool set_options(
    const char * filter_expression,
    size_t expression_parameter_argc,
    const char * expression_parameter_argv[])
  {
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    rmw_subscription_content_filter_options_t options =
      rmw_get_zero_initialized_content_filter_options();
    EXPECT_EQ(
      RMW_RET_OK,
      rmw_subscription_content_filter_options_init(
        filter_expression,
        expression_parameter_argc,
        expression_parameter_argv,
        &allocator,
        &options)
    );
    OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
    {
      rmw_subscription_content_filter_options_fini(&options, &allocator);
    });

    return common_content_filter_set(instance, &options);
  }

  void * instance;
  const rosidl_message_type_support_t * type_support;
};

class TestCommonContentFilterAPI : public ::testing::Test, public TestAPIBase
{
protected:
  void SetUp()
  {
    type_support = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
    instance = common_content_filter_create(type_support);
    EXPECT_NE(instance, nullptr);
  }

  void TearDown()
  {
    common_content_filter_destroy(instance);
  }

protected:
  const char * filter_expression = "int32_value = %0";
  std::vector<const char *> expression_parameter = {"4"};
};

TEST_F(TestCommonContentFilterAPI, is_enabled) {
  EXPECT_FALSE(common_content_filter_is_enabled(nullptr));
  EXPECT_FALSE(common_content_filter_is_enabled(instance));
  EXPECT_TRUE(
    set_options(filter_expression, expression_parameter.size(), expression_parameter.data()));
  EXPECT_TRUE(common_content_filter_is_enabled(instance));
}

TEST_F(TestCommonContentFilterAPI, evaluate) {
  EXPECT_FALSE(common_content_filter_evaluate(nullptr, nullptr, false));
  EXPECT_FALSE(common_content_filter_evaluate(instance, nullptr, false));
  EXPECT_FALSE(common_content_filter_evaluate(nullptr, nullptr, true));
  EXPECT_FALSE(common_content_filter_evaluate(instance, nullptr, true));

  test_msgs__msg__BasicTypes msg;
  test_msgs__msg__BasicTypes__init(&msg);
  msg.int32_value = 3;
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    test_msgs__msg__BasicTypes__fini(&msg);
  });

  // test serialized message
  rmw_serialized_message_t serialized_message =
    rmw_get_zero_initialized_serialized_message();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  EXPECT_EQ(
    RMW_RET_OK,
    rmw_serialized_message_init(&serialized_message, 1U, &allocator)
  );
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rmw_serialized_message_fini(&serialized_message);
  });

  EXPECT_EQ(
    RMW_RET_OK,
    rmw_serialize(&msg, type_support, &serialized_message)
  );

  // no filter setting, expect true
  EXPECT_TRUE(common_content_filter_evaluate(instance, &msg, false));
  EXPECT_TRUE(common_content_filter_evaluate(instance, &serialized_message, true));
  // after setting filter with "int32_value = 4"
  EXPECT_TRUE(
    set_options(filter_expression, expression_parameter.size(), expression_parameter.data()));
  // expect msg(int32_value = 3) return false
  EXPECT_FALSE(common_content_filter_evaluate(instance, &msg, false));
  EXPECT_FALSE(common_content_filter_evaluate(instance, &serialized_message, true));
  // update msg with 4
  msg.int32_value = 4;
  EXPECT_EQ(
    RMW_RET_OK,
    rmw_serialize(&msg, type_support, &serialized_message)
  );

  EXPECT_TRUE(common_content_filter_evaluate(instance, &msg, false));
  EXPECT_TRUE(common_content_filter_evaluate(instance, &serialized_message, true));
}

TEST_F(TestCommonContentFilterAPI, set) {
  EXPECT_FALSE(common_content_filter_set(nullptr, nullptr));
  EXPECT_FALSE(common_content_filter_set(instance, nullptr));
  EXPECT_TRUE(
    set_options(filter_expression, expression_parameter.size(), expression_parameter.data()));

  const char * filter_expression_error = "error_int32_value = %0";
  EXPECT_FALSE(
    set_options(filter_expression_error, expression_parameter.size(), expression_parameter.data()));
}

TEST_F(TestCommonContentFilterAPI, get) {
  EXPECT_FALSE(common_content_filter_get(nullptr, nullptr, nullptr));
  EXPECT_FALSE(common_content_filter_get(instance, nullptr, nullptr));
  EXPECT_TRUE(
    set_options(filter_expression, expression_parameter.size(), expression_parameter.data()));
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  EXPECT_FALSE(common_content_filter_get(instance, &allocator, nullptr));

  rmw_subscription_content_filter_options_t options =
    rmw_get_zero_initialized_content_filter_options();
  EXPECT_TRUE(common_content_filter_get(instance, &allocator, &options));
  OSRF_TESTING_TOOLS_CPP_SCOPE_EXIT(
  {
    rmw_subscription_content_filter_options_fini(&options, &allocator);
  });

  EXPECT_STREQ(options.filter_expression, filter_expression);
  ASSERT_EQ(expression_parameter.size(), options.expression_parameters.size);
  for (size_t i = 0; i < expression_parameter.size(); ++i) {
    EXPECT_STREQ(options.expression_parameters.data[i], expression_parameter[i]);
  }
}

class TestComplexMsgCommonContentFilterAPI : public ::testing::Test, public TestAPIBase
{
protected:
  void SetUp()
  {
    type_support =
      rosidl_typesupport_cpp::get_message_type_support_handle<
      test_content_filter_msgs::msg::Complex>();

    instance = common_content_filter_create(type_support);
    EXPECT_NE(instance, nullptr);
  }

  void TearDown()
  {
    common_content_filter_destroy(instance);
  }
};

TEST_F(TestComplexMsgCommonContentFilterAPI, set_and_evaluate) {
  // default is zero
  test_msgs::msg::BasicTypes basic_types_data_zero;

  // set member data with one
  test_msgs::msg::BasicTypes basic_types_data_one;
  basic_types_data_one.bool_value = true;
  basic_types_data_one.byte_value = 1;
  basic_types_data_one.char_value = 1;
  basic_types_data_one.float32_value = 1.0f;
  basic_types_data_one.float64_value = 1.0;
  basic_types_data_one.int8_value = 1;
  basic_types_data_one.uint8_value = 1;
  basic_types_data_one.int16_value = 1;
  basic_types_data_one.uint16_value = 1;
  basic_types_data_one.int32_value = 1l;
  basic_types_data_one.uint32_value = 1ul;
  basic_types_data_one.int64_value = 1ll;
  basic_types_data_one.uint64_value = 1ull;

  test_content_filter_msgs::msg::Basic basic_zero_one;
  basic_zero_one.names = {
    "basic_zero_one_first_name",            // data.basic_array[0].names[0]
    "basic_zero_one_second_name"            // data.basic_array[0].names[1]
  };
  basic_zero_one.basic_types = {
    basic_types_data_zero,                  // data.basic_array[0].basic_types[0] is 0
    basic_types_data_one                    // data.basic_array[0].basic_types[1] is 1
  };
  basic_zero_one.unbounded_int32_data = {0, 1};
  basic_zero_one.bounded_float64_data = {0, 1};

  test_content_filter_msgs::msg::Basic basic_one_zero;
  basic_one_zero.names = {
    "basic_one_zero_first_name",            // data.basic_array[1].names[0]
    "basic_one_zero_second_name"            // data.basic_array[1].names[1]
  };
  basic_one_zero.basic_types = {
    basic_types_data_one,                   // data.basic_array[1].basic_types[0] is 1
    basic_types_data_zero                   // data.basic_array[1].basic_types[1] is 0
  };
  basic_one_zero.unbounded_int32_data = {1, 0};
  basic_one_zero.bounded_float64_data = {1, 0};

  test_content_filter_msgs::msg::Complex msg;
  msg.data.basic_array = {
    basic_zero_one,
    basic_one_zero
  };
  msg.data.names = {
    "intermedia_first_name",                // data.names[0]
    "intermedia_second_name"                // data.names[1]
  };

  msg.name = "complex_name";                // name

  // no filter, expect the true value by evaluating a message
  EXPECT_TRUE(common_content_filter_evaluate(instance, &msg, false));

  struct Info
  {
    const char * filter_expression;
    std::vector<const char *> expression_parameter;
    bool set_expectation;
    bool evaluate_expectation;
  };

  std::vector<Info> expectation = {
    // name with string or string array
    {"name=%0", {"'complex_name'"}, true, true},
    {"name=%0", {"'not_complex_name'"}, true, false},

    {"data.names[0]=%0", {"'intermedia_first_name'"}, true, true},
    {"data.names[0]=%0", {"'intermedia_second_name'"}, true, false},
    {"data.names[1]=%0", {"'intermedia_first_name'"}, true, false},
    {"data.names[1]=%0", {"'intermedia_second_name'"}, true, true},

    {"data.basic_array[0].names[0]=%0", {"'basic_zero_one_first_name'"}, true, true},
    {"data.basic_array[0].names[0]=%0", {"'basic_zero_one_second_name'"}, true, false},
    {"data.basic_array[0].names[1]=%0", {"'basic_zero_one_first_name'"}, true, false},
    {"data.basic_array[0].names[1]=%0", {"'basic_zero_one_second_name'"}, true, true},

    {"data.basic_array[1].names[0]=%0", {"'basic_one_zero_first_name'"}, true, true},
    {"data.basic_array[1].names[0]=%0", {"'basic_one_zero_second_name'"}, true, false},
    {"data.basic_array[1].names[1]=%0", {"'basic_one_zero_first_name'"}, true, false},
    {"data.basic_array[1].names[1]=%0", {"'basic_one_zero_second_name'"}, true, true},

    {"name=%0 and data.names[0]=%1", {"'complex_name'", "'intermedia_first_name'"}, true, true},
    {"name=%0 and data.names[0]=%1", {"'not_complex_name'", "'intermedia_first_name'"}, true,
      false},
    {"name=%0 or data.names[0]=%1", {"'complex_name'", "'intermedia_first_name'"}, true, true},
    {"name=%0 or data.names[0]=%1", {"'not_complex_name'", "'intermedia_first_name'"}, true, true},
    {"name=%0 or data.names[0]=%1", {"'complex_name'", "'intermedia_second_name'"}, true, true},

    // basic types array
    // [0] [0]
    {"data.basic_array[0].basic_types[0].bool_value=%0", {"false"}, true, true},
    {"data.basic_array[0].basic_types[0].byte_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].char_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].float32_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].float64_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].int8_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].uint8_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].int16_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].uint16_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].int32_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].uint32_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].int64_value=%0", {"0"}, true, true},
    {"data.basic_array[0].basic_types[0].uint64_value=%0", {"0"}, true, true},
    {"data.basic_array[0].unbounded_int32_data[0]=%0", {"0"}, true, true},
    {"data.basic_array[0].bounded_float64_data[0]=%0", {"0"}, true, true},

    {"data.basic_array[0].basic_types[0].bool_value=%0", {"true"}, true, false},
    {"data.basic_array[0].basic_types[0].byte_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].char_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].float32_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].float64_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].int8_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].uint8_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].int16_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].uint16_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].int32_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].uint32_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].int64_value=%0", {"1"}, true, false},
    {"data.basic_array[0].basic_types[0].uint64_value=%0", {"1"}, true, false},
    {"data.basic_array[0].unbounded_int32_data[0]=%0", {"1"}, true, false},
    {"data.basic_array[0].bounded_float64_data[0]=%0", {"1"}, true, false},

    // [0] [1]
    {"data.basic_array[0].basic_types[1].bool_value=%0", {"false"}, true, false},
    {"data.basic_array[0].basic_types[1].byte_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].char_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].float32_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].float64_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].int8_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].uint8_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].int16_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].uint16_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].int32_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].uint32_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].int64_value=%0", {"0"}, true, false},
    {"data.basic_array[0].basic_types[1].uint64_value=%0", {"0"}, true, false},
    {"data.basic_array[0].unbounded_int32_data[1]=%0", {"0"}, true, false},
    {"data.basic_array[0].bounded_float64_data[1]=%0", {"0"}, true, false},

    {"data.basic_array[0].basic_types[1].bool_value=%0", {"true"}, true, true},
    {"data.basic_array[0].basic_types[1].byte_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].char_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].float32_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].float64_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].int8_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].uint8_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].int16_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].uint16_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].int32_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].uint32_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].int64_value=%0", {"1"}, true, true},
    {"data.basic_array[0].basic_types[1].uint64_value=%0", {"1"}, true, true},
    {"data.basic_array[0].unbounded_int32_data[1]=%0", {"1"}, true, true},
    {"data.basic_array[0].bounded_float64_data[1]=%0", {"1"}, true, true},

    // [1][0]
    {"data.basic_array[1].basic_types[0].bool_value=%0", {"false"}, true, false},
    {"data.basic_array[1].basic_types[0].byte_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].char_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].float32_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].float64_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].int8_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].uint8_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].int16_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].uint16_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].int32_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].uint32_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].int64_value=%0", {"0"}, true, false},
    {"data.basic_array[1].basic_types[0].uint64_value=%0", {"0"}, true, false},
    {"data.basic_array[1].unbounded_int32_data[0]=%0", {"0"}, true, false},
    {"data.basic_array[1].bounded_float64_data[0]=%0", {"0"}, true, false},

    {"data.basic_array[1].basic_types[0].bool_value=%0", {"true"}, true, true},
    {"data.basic_array[1].basic_types[0].byte_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].char_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].float32_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].float64_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].int8_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].uint8_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].int16_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].uint16_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].int32_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].uint32_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].int64_value=%0", {"1"}, true, true},
    {"data.basic_array[1].basic_types[0].uint64_value=%0", {"1"}, true, true},
    {"data.basic_array[1].unbounded_int32_data[0]=%0", {"1"}, true, true},
    {"data.basic_array[1].bounded_float64_data[0]=%0", {"1"}, true, true},

    // [1][1]
    {"data.basic_array[1].basic_types[1].bool_value=%0", {"false"}, true, true},
    {"data.basic_array[1].basic_types[1].byte_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].char_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].float32_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].float64_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].int8_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].uint8_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].int16_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].uint16_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].int32_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].uint32_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].int64_value=%0", {"0"}, true, true},
    {"data.basic_array[1].basic_types[1].uint64_value=%0", {"0"}, true, true},
    {"data.basic_array[1].unbounded_int32_data[1]=%0", {"0"}, true, true},
    {"data.basic_array[1].bounded_float64_data[1]=%0", {"0"}, true, true},

    {"data.basic_array[1].basic_types[1].bool_value=%0", {"true"}, true, false},
    {"data.basic_array[1].basic_types[1].byte_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].char_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].float32_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].float64_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].int8_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].uint8_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].int16_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].uint16_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].int32_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].uint32_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].int64_value=%0", {"1"}, true, false},
    {"data.basic_array[1].basic_types[1].uint64_value=%0", {"1"}, true, false},
    {"data.basic_array[1].unbounded_int32_data[1]=%0", {"1"}, true, false},
    {"data.basic_array[1].bounded_float64_data[1]=%0", {"1"}, true, false},

    // some other cases
    // bad field name
    {"error_name=%0", {"'complex_name'"}, false, true},
    {"errordata.names[0]=%0", {"'intermedia_first_name'"}, false, true},

    // unbound case
    {"data.names[10]=%0", {"'unbound_name'"}, true, false},

    // bound case, the size of bounded_float64_data is 2
    {"data.basic_array[0].bounded_float64_data[10]=%0", {"0"}, false, true},

    // TODO(iuhilnehc-ynos): if bugs found, add new test cases and fix source code.
  };

  for (auto & item : expectation) {
    EXPECT_EQ(
      item.set_expectation,
      set_options(
        item.filter_expression,
        item.expression_parameter.size(),
        item.expression_parameter.data()))
      << "Set error happened by the filter expression:" << item.filter_expression;
    EXPECT_EQ(item.evaluate_expectation, common_content_filter_evaluate(instance, &msg, false))
      << "Evaluate error happened by the filter expression:" << item.filter_expression;

    // reset
    EXPECT_TRUE(set_options("", 0, nullptr));
  }
}
