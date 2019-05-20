# pthread
if(UNIX) # include Linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

set(examples
    binary_example
    interface_example
    resolver_example
    tcp_client_example
    tcp_server_example
    udp_client_example
    udp_server_example
)
if(NET_USE_OPENSSL)
    set(examples ${examples}
        https_client_example
        ssl_client_example
        ssl_server_example
    )
endif()

project_add_example(
    ${examples}
)
