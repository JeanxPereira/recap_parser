cmake_minimum_required(VERSION 3.20)
project(recap_parser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

if(MSVC)
  add_compile_options(/utf-8)
endif()

include(FetchContent)
FetchContent_Declare(
  CPM
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake
  GIT_TAG origin/master)
FetchContent_MakeAvailable(CPM)

set(BOOST_INCLUDE_LIBRARIES "program_options;filesystem")
CPMAddPackage(
  NAME Boost
  VERSION 1.84.0
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
  URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
  OPTIONS "BOOST_ENABLE_CMAKE ON")

CPMAddPackage("gh:fmtlib/fmt#10.2.1")

CPMAddPackage("gh:jbeder/yaml-cpp#0.8.0")

CPMAddPackage("gh:zeux/pugixml#v1.13")

add_executable(recap_parser
  main.cpp 
  catalog.cpp
  parser.cpp
  Resource.rc
)

target_include_directories(recap_parser PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(recap_parser PRIVATE
  -Wl,-Bstatic
  pugixml
  Boost::program_options
  Boost::filesystem
  fmt::fmt
  yaml-cpp
  -Wl,-Bdynamic
  -ldl -lpthread -lm -lrt
)

install(TARGETS recap_parser DESTINATION bin)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/icon1.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  add_custom_command(TARGET recap_parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE_DIR:recap_parser>
      $<TARGET_FILE:pugixml>
      $<TARGET_FILE:yaml-cpp>
      $<TARGET_FILE:fmt::fmt>
  )
endif()
