include(CMakeParseArguments)

# project_add_example(<module_name>
#                        [DEPENDS [depends1 [depends2 ...]]]
#                        example_name1 [example_name2 ...])
macro(project_add_example)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    include_directories(
        ${PROJECT_SOURCE_DIR}/include
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
