cmake_minimum_required(VERSION 3.8)

project(rcl_content_filter_fallback)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rcutils REQUIRED)
find_package(rcpputils REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_implementation REQUIRED)
find_package(rosidl_typesupport_introspection_c REQUIRED)
find_package(rosidl_typesupport_introspection_cpp REQUIRED)
find_package(tao_pegtl_vendor REQUIRED)
find_package(pegtl REQUIRED)

add_library(${PROJECT_NAME} SHARED
  src/FilterCompoundCondition.cpp
  src/FilterExpression.cpp
  src/FilterExpressionParser.cpp
  src/FilterFactory.cpp
  src/FilterField.cpp
  src/FilterParameter.cpp
  src/FilterPredicate.cpp
  src/FilterValue.cpp
  src/Utilities.cpp
  src/rcl_content_filter_fallback.cpp)

set_target_properties(${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")

target_link_libraries(${PROJECT_NAME} PUBLIC
  ament_index_cpp::ament_index_cpp
  rcpputils::rcpputils
  rcutils::rcutils
  rmw::rmw
  rmw_implementation::rmw_implementation
  rosidl_typesupport_introspection_c::rosidl_typesupport_introspection_c
  rosidl_typesupport_introspection_cpp::rosidl_typesupport_introspection_cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  taocpp::pegtl
)

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME}
  PRIVATE "RCL_CONTENT_FILTER_FALLBACK_BUILDING_LIBRARY")

install(
  TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Export old-style CMake variables
ament_export_include_directories("include/${PROJECT_NAME}")
ament_export_libraries(${PROJECT_NAME})

# Export modern CMake targets
ament_export_targets(${PROJECT_NAME})

ament_export_dependencies(ament_cmake_core)
ament_export_dependencies(rcpputils)
ament_export_dependencies(rcutils)
ament_export_dependencies(rmw)
ament_export_dependencies(rmw_implementation)
ament_export_dependencies(rosidl_typesupport_introspection_c)
ament_export_dependencies(rosidl_typesupport_introspection_cpp)
ament_export_dependencies(tao_pegtl_vendor)

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  find_package(test_content_filter_msgs REQUIRED)
  find_package(test_msgs REQUIRED)
  find_package(osrf_testing_tools_cpp REQUIRED)

  file(GLOB_RECURSE EXCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FilterExpressionParserImpl/*.hpp)
  set(ament_cmake_cpplint_ADDITIONAL_EXCLUDE ${EXCLUDE_FILES})
  ament_lint_auto_find_test_dependencies()

  ament_add_gtest(test_api test/test_api.cpp)
  target_link_libraries(test_api ${PROJECT_NAME})
  ament_target_dependencies(
    test_api test_content_filter_msgs test_msgs osrf_testing_tools_cpp)
endif()

ament_package()
