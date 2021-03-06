cmake_minimum_required(VERSION 2.8.7)
project(netlib CXX)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

option(NETLIB_USE_OPENSSL "Include SSL/TLS support" OFF)

option(build_tests "Build all of own tests" OFF)
option(build_examples "Build example programs" OFF)

include(cmake/project.cmake)

### Library
include_directories(
    ${PROJECT_SOURCE_DIR}/src
)
set(source_files
    ${PROJECT_SOURCE_DIR}/src/netlib/error.cpp
    ${PROJECT_SOURCE_DIR}/src/netlib/binary.cpp
    ${PROJECT_SOURCE_DIR}/src/netlib/interface.cpp
    ${PROJECT_SOURCE_DIR}/src/netlib/resolver.cpp
    ${PROJECT_SOURCE_DIR}/src/netlib/stream.cpp
)
if(WIN32)
    set(source_files ${source_files}
        ${PROJECT_SOURCE_DIR}/src/netlib/interface_windows.cpp
        ${PROJECT_SOURCE_DIR}/src/netlib/internal/init_windows.cpp
        ${PROJECT_SOURCE_DIR}/src/netlib/tcp_windows.cpp
        ${PROJECT_SOURCE_DIR}/src/netlib/udp_windows.cpp
    )
else() # UNIX
    set(source_files ${source_files}
        ${PROJECT_SOURCE_DIR}/src/netlib/internal/init_unix.cpp
        ${PROJECT_SOURCE_DIR}/src/netlib/tcp_unix.cpp
        ${PROJECT_SOURCE_DIR}/src/netlib/udp_unix.cpp
    )
    if(APPLE)
        set(source_files ${source_files}
            ${PROJECT_SOURCE_DIR}/src/netlib/interface_bsd.cpp
        )
    else() # Linux
        set(source_files ${source_files}
            ${PROJECT_SOURCE_DIR}/src/netlib/interface_linux.cpp
        )
    endif()
endif()
if(NETLIB_USE_OPENSSL)
    set(source_files ${source_files}
        ${PROJECT_SOURCE_DIR}/src/netlib/ssl.cpp
    )
endif()
# shared and static libraries
add_library(${PROJECT_NAME} SHARED ${source_files})
add_library(${PROJECT_NAME}_static STATIC ${source_files})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}_static PROPERTIES PREFIX "lib")
if(MINGW)
    target_link_libraries(${PROJECT_NAME} ws2_32 iphlpapi)
    target_link_libraries(${PROJECT_NAME}_static ws2_32 iphlpapi)
endif()
if(NETLIB_USE_OPENSSL)
    target_link_libraries(${PROJECT_NAME} ssl crypto)
    target_link_libraries(${PROJECT_NAME}_static ssl crypto)
endif()

### Install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
# install header files
file(GLOB header_files ${PROJECT_SOURCE_DIR}/src/netlib/*.h)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/include/netlib)
install(FILES ${header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/netlib)
# install internal header files
file(GLOB internal_header_files ${PROJECT_SOURCE_DIR}/src/netlib/internal/*.h)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/include/netlib/internal)
install(FILES ${internal_header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/netlib/internal)

### Test
if(build_tests)
    enable_testing()
    set(BUILD_GTEST ON)
    set(BUILD_GMOCK OFF)
    add_subdirectory(vendor/googletest)
    add_subdirectory(test)
endif()

### Example
if(build_examples)
    add_subdirectory(example)
endif()
