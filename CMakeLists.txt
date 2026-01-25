cmake_minimum_required(VERSION 3.30)
project(recap_parser LANGUAGES C CXX RC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RECAP_ENABLE_LTO "Enable IPO/LTO" ON)
option(RECAP_FULL_STATIC "Link statically on MinGW" ON)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)
set(CPM_DOWNLOAD_VERSION 0.42.0)
set(_cpm "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
if(NOT EXISTS "${_cpm}")
  file(DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake" "${_cpm}" TLS_VERIFY ON)
endif()
include("${_cpm}")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

CPMAddPackage(NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 11.2.0)

CPMAddPackage(NAME CLI11
  GITHUB_REPOSITORY CLIUtils/CLI11
  GIT_TAG v2.5.0
  OPTIONS "CLI11_PRECOMPILED OFF")

CPMAddPackage(NAME pugixml
  GITHUB_REPOSITORY zeux/pugixml
  GIT_TAG v1.15
  OPTIONS "PUGIXML_BUILD_TESTS OFF" "PUGIXML_BUILD_SAMPLES OFF")

CPMAddPackage(NAME yaml-cpp
  GITHUB_REPOSITORY jbeder/yaml-cpp
  GIT_TAG c268020
  OPTIONS "YAML_CPP_BUILD_TESTS OFF" "YAML_CPP_BUILD_TOOLS OFF" "YAML_BUILD_SHARED_LIBS OFF")

set(src
  main.cpp
  parser.cpp
  catalog.cpp
)

if(WIN32)
  list(APPEND src Resource.rc)
endif()

add_executable(recap_parser ${src})

target_link_libraries(recap_parser
  PRIVATE
    fmt::fmt
    CLI11::CLI11
    yaml-cpp
    pugixml
)

target_include_directories(recap_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  target_compile_options(recap_parser PRIVATE /W4 /permissive- /Zc:preprocessor)
else()
  target_compile_options(recap_parser PRIVATE -Wall -Wextra -Wpedantic -fno-omit-frame-pointer)
endif()

if(RECAP_ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT _ipo)
  if(_ipo)
    set_property(TARGET recap_parser PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

if(MINGW AND RECAP_FULL_STATIC)
  target_link_options(recap_parser PRIVATE -static -static-libgcc -static-libstdc++)
endif()

set_target_properties(recap_parser PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
