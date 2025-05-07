cmake_minimum_required(VERSION 3.14)
project(recap_parser VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
# If running on Windows, add UTF-8 support similar to the /utf-8 MSVC option
if(MSVC)
  add_compile_options(/utf-8)
endif()

# Use vcpkg toolchain file if it's specified or can be found
if(DEFINED CMAKE_TOOLCHAIN_FILE)
  # Toolchain file was specified on command line, use it
elseif(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Find required packages
find_package(PugiXML REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options filesystem)
find_package(fmt CONFIG REQUIRED)
find_package(yaml-cpp REQUIRED)

# Add the executable
add_executable(recap_parser 
  main.cpp 
  catalog.cpp
  parser.cpp
  Resource.rc
)

# Include directories
target_include_directories(recap_parser PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(recap_parser PRIVATE
  pugixml::pugixml
  Boost::program_options
  Boost::filesystem
  fmt::fmt
  yaml-cpp
)

# Install
install(TARGETS recap_parser DESTINATION bin)

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy icon files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/icon1.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# If Windows, copy DLLs to output directory
if(WIN32)
  add_custom_command(TARGET recap_parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:pugixml::pugixml>
      $<TARGET_FILE:yaml-cpp::yaml-cpp>
      $<TARGET_FILE:fmt::fmt>
      $<TARGET_FILE_DIR:recap_parser>
  )
endif()