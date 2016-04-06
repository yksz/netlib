include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

set(examples
    binary_example
)

foreach(example IN LISTS examples)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} ${PROJECT_NAME}_static)
endforeach()
