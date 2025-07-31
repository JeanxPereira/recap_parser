cmake_minimum_required(VERSION 3.14)
project(recap_parser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_STATIC_EXECUTABLE "Build a static executable" ON)

if(BUILD_STATIC_EXECUTABLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
    
    if(UNIX AND NOT APPLE)
        include(CheckCXXLinkerFlag)
        check_cxx_linker_flag("-static" STATIC_LINKING_SUPPORTED)
        
        if(STATIC_LINKING_SUPPORTED)
            message(STATUS "Static linking supported, but using selective approach")
        endif()
    endif()
else()
    set(BUILD_SHARED_LIBS ON)
endif()

if(MSVC)
  add_compile_options(/utf-8)
endif()

if(DEFINED CMAKE_TOOLCHAIN_FILE)
elseif(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(BUILD_STATIC_EXECUTABLE)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
endif()

find_package(PkgConfig QUIET)
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

function(target_link_libraries_selective target)
    if(BUILD_STATIC_EXECUTABLE)
        target_link_libraries(${target} PRIVATE
            -Wl,-Bstatic
            pugixml::pugixml
            Boost::program_options
            Boost::filesystem  
            fmt::fmt
            yaml-cpp
            -Wl,-Bdynamic
            ${CMAKE_THREAD_LIBS_INIT}
        )
        
        if(UNIX AND NOT APPLE)
            target_link_libraries(${target} PRIVATE -lm -ldl -lrt)
        endif()
    else()
        target_link_libraries(${target} PRIVATE
            pugixml::pugixml
            Boost::program_options
            Boost::filesystem
            fmt::fmt
            yaml-cpp
        )
    endif()
endfunction()

target_link_libraries_selective(recap_parser)

if(PkgConfig_FOUND AND BUILD_STATIC_EXECUTABLE)
    # Tentar usar pkg-config para bibliotecas que suportam
    pkg_check_modules(PC_FMT QUIET fmt)
    if(PC_FMT_FOUND)
        # Usar variáveis STATIC quando disponíveis
        if(PC_FMT_STATIC_LIBRARIES)
            message(STATUS "Using static fmt via pkg-config")
            target_link_libraries(recap_parser PRIVATE ${PC_FMT_STATIC_LIBRARIES})
        endif()
    endif()
endif()

get_target_property(RECAP_LINK_LIBRARIES recap_parser LINK_LIBRARIES)
message(STATUS "Final link libraries: ${RECAP_LINK_LIBRARIES}")

if(BUILD_STATIC_EXECUTABLE)
    message(STATUS "Building static executable")
    message(STATUS "CMAKE_FIND_LIBRARY_SUFFIXES: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
else()
    message(STATUS "Building dynamic executable")
endif()

install(TARGETS recap_parser DESTINATION bin)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/icon1.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})