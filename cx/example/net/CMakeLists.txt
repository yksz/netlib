# pthread
if(UNIX) # include Linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

project_add_example(
    interface_example
    lookup_example
    tcp_client_example
    tcp_server_example
    udp_client_example
    udp_server_example
)
