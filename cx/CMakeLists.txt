cmake_minimum_required(VERSION 2.8.7)
project(cxlib CXX)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

option(build_tests "Build all of own tests." OFF)
option(build_examples "Build example programs." OFF)

include(cmake/project.cmake)

### Library
include_directories(
    ${PROJECT_SOURCE_DIR}/src
)
set(source_files
    ${PROJECT_SOURCE_DIR}/src/cx/io/binary.cpp
    ${PROJECT_SOURCE_DIR}/src/cx/io/stream.cpp
)
if(WIN32)
    set(source_files ${source_files}
        ${PROJECT_SOURCE_DIR}/src/cx/net/windows/tcp_windows.cpp
    )
else()
    set(source_files ${source_files}
        ${PROJECT_SOURCE_DIR}/src/cx/net/unix/tcp_unix.cpp
    )
endif()
# shared and static libraries
add_library(${PROJECT_NAME} SHARED ${source_files})
add_library(${PROJECT_NAME}_static STATIC ${source_files})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(UNIX)
    set_target_properties(${PROJECT_NAME}_static PROPERTIES PREFIX "lib")
endif()
if(MINGW)
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME}_static ws2_32)
endif()

### Install

### Test
#if(build_tests)
#    enable_testing()
#    add_subdirectory(test)
#endif()

### Example
if(build_examples)
    add_subdirectory(example)
endif()
