set(tests
    binary_test
    resolver_test
)
if(UNIX AND APPLE)
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

    DEPENDS
    ${PROJECT_NAME}
)
