include(CMakeParseArguments)

# project_add_googletest([DEPENDS [depends1 [depends2 ...]]]
#                        test_name1 [test_name2 ...])
macro(project_add_googletest)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    include_directories(
        ${PROJECT_SOURCE_DIR}/vendor/googletest/googletest/include
#        /usr/local/include # if googletest is installed
    )
    link_directories(
#        /usr/local/lib # if googletest is installed
    )
    set(test_libraries ${arg_DEPENDS}
        gtest
        gtest_main
    )
if(WIN32)
    set(test_libraries ${test_libraries}
        ${PROJECT_NAME}_static
    )
else() # UNIX
    set(test_libraries ${test_libraries}
        ${PROJECT_NAME}
        pthread
    )
endif()
    foreach(test_path IN LISTS arg_UNPARSED_ARGUMENTS)
        string(REPLACE "/" "_" test_name ${test_path})
        set(test ${test_name})
        add_executable(${test} ${test_path}.cpp)
        target_link_libraries(${test} ${test_libraries})
        add_test(${test} ${test})
    endforeach()
endmacro(project_add_googletest)

# project_add_example([DEPENDS [depends1 [depends2 ...]]]
#                     example_name1 [example_name2 ...])
macro(project_add_example)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    include_directories(
    )
    set(example_libraries ${arg_DEPENDS}
        ${PROJECT_NAME}_static
    )
    foreach(example_path IN LISTS arg_UNPARSED_ARGUMENTS)
        string(REPLACE "/" "_" example_name ${example_path})
        set(example ${example_name})
        add_executable(${example} ${example_path}.cpp)
        target_link_libraries(${example} ${example_libraries})
    endforeach()
endmacro(project_add_example)
