set(tests
    binary_test
    resolver_test
    tcp_test
    udp_test
)
if(WIN32)
    set(tests ${tests}
    )
elseif(UNIX AND APPLE)
    set(tests ${tests}
        interface_bsd_test
    )
else()
    set(tests ${tests}
        interface_linux_test
    )
endif()

project_add_googletest(
    ${tests}
)
