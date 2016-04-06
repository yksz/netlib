# pthread
if(UNIX) # include Linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

set(examples
    tcp_client_example
    tcp_server_example
)

foreach(example IN LISTS examples)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} ${PROJECT_NAME}_static)
endforeach()
