# Add test_interchip example
add_executable(test_interchip ${CMAKE_CURRENT_SOURCE_DIR}/combined_payloads/test_interchip.c)
target_link_libraries(test_interchip PRIVATE ducky_common)
target_include_directories(test_interchip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common/include)

# Add to the list of examples
list(APPEND EXAMPLE_TARGETS test_interchip)