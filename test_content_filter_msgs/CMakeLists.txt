cmake_minimum_required(VERSION 3.5)

project(test_content_filter_msgs)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(test_msgs REQUIRED)

rosidl_generate_interfaces(test_content_filter_msgs
  "msg/Basic.msg"
  "msg/Complex.msg"
  "msg/Intermediate.msg"
  DEPENDENCIES test_msgs
)

ament_package()
