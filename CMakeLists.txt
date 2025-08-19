cmake_minimum_required(VERSION 3.20)
project(recap_parser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

if(MSVC)
  add_compile_options(/utf-8)
endif()

include(FetchContent)
FetchContent_Declare(
  CPM
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake
  GIT_TAG origin/master)
FetchContent_MakeAvailable(CPM)

CPMAddPackage(
  NAME CLI11
  GITHUB_REPOSITORY CLIUtils/CLI11
  VERSION 2.4.2
)

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
  CLI11::CLI11
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
