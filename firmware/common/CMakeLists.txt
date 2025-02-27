cmake_minimum_required(VERSION 3.16)
project(DuckyCommon VERSION 0.1.0 LANGUAGES C CXX)

# Include CMake package helper modules
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Define the library for common code shared between both controllers
add_library(ducky_common STATIC
    src/protocol.c
    src/security.c
    src/payload_framework.c
)

# Include directories
target_include_directories(ducky_common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/cpp>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cpp>
)

# Make the C++ wrapper depend on the C library
target_link_libraries(ducky_common_cpp INTERFACE ducky_common)

# Set properties for better IDE integration
set_target_properties(ducky_common PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# Install header files to include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cpp/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cpp
        FILES_MATCHING PATTERN "*.hpp"
)

# Export our library targets
install(TARGETS ducky_common ducky_common_cpp
        EXPORT DuckyCommonTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Configure version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Create config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DuckyCommonConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DuckyCommon
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DuckyCommonConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DuckyCommon
)

# Install export targets
install(EXPORT DuckyCommonTargets
    FILE DuckyCommonTargets.cmake
    NAMESPACE DuckyCommon::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DuckyCommon
)

# Generate compile_commands.json for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add unittest target if testing is enabled
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()