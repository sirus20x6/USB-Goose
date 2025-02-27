cmake_minimum_required(VERSION 3.16)
project(DuckyCommon VERSION 0.1.0 LANGUAGES C CXX)

# Define the library for common code shared between both controllers
add_library(ducky_common STATIC
    src/protocol.c
    src/security.c
    src/payload_framework.c
)

# Include directories
target_include_directories(ducky_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Path to project root for accessing other module includes
if(DEFINED PROJECT_ROOT)
    target_include_directories(ducky_common PUBLIC
        ${PROJECT_ROOT}/common/include
    )
endif()

# Create a header-only C++ wrapper for the C interfaces
add_library(ducky_common_cpp INTERFACE)
target_include_directories(ducky_common_cpp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cpp
)

# Make the C++ wrapper depend on the C library
target_link_libraries(ducky_common_cpp INTERFACE ducky_common)

# Install header files to include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cpp/
        DESTINATION include/cpp
        FILES_MATCHING PATTERN "*.hpp"
)

# Export our library targets
install(TARGETS ducky_common ducky_common_cpp
        EXPORT DuckyCommonTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# Configure version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Create config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DuckyCommonConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfig.cmake
    INSTALL_DESTINATION lib/cmake/DuckyCommon
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfigVersion.cmake
    DESTINATION lib/cmake/DuckyCommon
)

# Install export targets
install(EXPORT DuckyCommonTargets
    FILE DuckyCommonTargets.cmake
    NAMESPACE DuckyCommon::
    DESTINATION lib/cmake/DuckyCommon
)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add unittest target if testing is enabled
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()