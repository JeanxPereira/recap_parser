cmake_minimum_required(VERSION 3.14)
project(recap_parser VERSION 1.0.0 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FORÇA LINKAGEM TOTALMENTE ESTÁTICA
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc")

if(MSVC)
  add_compile_options(/utf-8)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(DEFINED CMAKE_TOOLCHAIN_FILE)
elseif(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(PugiXML REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options filesystem)
find_package(fmt CONFIG REQUIRED)
find_package(yaml-cpp REQUIRED)

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
  pugixml::pugixml
  Boost::program_options
  Boost::filesystem
  fmt::fmt
  yaml-cpp
  -lpthread
  -lm
  -ldl
  -lrt
)

install(TARGETS recap_parser DESTINATION bin)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/icon1.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  add_custom_command(TARGET recap_parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:pugixml::pugixml>
      $<TARGET_FILE:yaml-cpp::yaml-cpp>
      $<TARGET_FILE:fmt::fmt>
      $<TARGET_FILE_DIR:recap_parser>
  )
endif()