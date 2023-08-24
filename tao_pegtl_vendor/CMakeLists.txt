cmake_minimum_required(VERSION 3.10)

project(tao_pegtl_vendor)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_vendor_package REQUIRED)

# build pegtl 2.8.3 if it's not available in the system or ros 2 environment
find_package(pegtl 2.8.3 QUIET)

set(BUILD_SHARED_LIBS TRUE CACHE BOOL "Build shared libraries")

ament_vendor(pegtl_vendor
  SATISFIED ${pegtl_FOUND}
  VCS_URL https://github.com/taocpp/PEGTL.git
  VCS_VERSION 2.8.3
  CMAKE_ARGS
    -DPEGTL_BUILD_TESTS:BOOL=OFF
    -DPEGTL_BUILD_EXAMPLES:BOOL=OFF
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  list(APPEND AMENT_LINT_AUTO_EXCLUDE
    ament_cmake_cppcheck
  )

  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
