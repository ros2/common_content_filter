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

#ifndef COMMON_CONTENT_FILTER__VISIBILITY_CONTROL_H_
#define COMMON_CONTENT_FILTER__VISIBILITY_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define COMMON_CONTENT_FILTER_EXPORT __attribute__ ((dllexport))
    #define COMMON_CONTENT_FILTER_IMPORT __attribute__ ((dllimport))
  #else
    #define COMMON_CONTENT_FILTER_EXPORT __declspec(dllexport)
    #define COMMON_CONTENT_FILTER_IMPORT __declspec(dllimport)
  #endif
  #ifdef COMMON_CONTENT_FILTER_BUILDING_LIBRARY
    #define COMMON_CONTENT_FILTER_PUBLIC COMMON_CONTENT_FILTER_EXPORT
  #else
    #define COMMON_CONTENT_FILTER_PUBLIC COMMON_CONTENT_FILTER_IMPORT
  #endif
  #define COMMON_CONTENT_FILTER_PUBLIC_TYPE COMMON_CONTENT_FILTER_PUBLIC
  #define COMMON_CONTENT_FILTER_LOCAL
#else
  #define COMMON_CONTENT_FILTER_EXPORT __attribute__ ((visibility("default")))
  #define COMMON_CONTENT_FILTER_IMPORT
  #if __GNUC__ >= 4
    #define COMMON_CONTENT_FILTER_PUBLIC __attribute__ ((visibility("default")))
    #define COMMON_CONTENT_FILTER_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define COMMON_CONTENT_FILTER_PUBLIC
    #define COMMON_CONTENT_FILTER_LOCAL
  #endif
  #define COMMON_CONTENT_FILTER_PUBLIC_TYPE
#endif

#ifdef __cplusplus
}
#endif

#endif  // COMMON_CONTENT_FILTER__VISIBILITY_CONTROL_H_
