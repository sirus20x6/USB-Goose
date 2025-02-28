cmake_minimum_required(VERSION 3.16)
project(USBGoose VERSION 0.2.0 LANGUAGES C CXX ASM)

# Enable verbose output for debugging if needed
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Options to control which components to build
option(BUILD_CH569 "Build CH569 USB controller firmware" ON)
option(BUILD_ESP32C6 "Build ESP32-C6 wireless controller firmware" ON)
option(BUILD_TOOLS "Build development tools" ON)
option(BUILD_EXAMPLES "Build example payloads" ON)
option(BUILD_DOCS "Build documentation" ON)
option(BUILD_TESTING "Build and run tests" ON)

# Additional language options
option(ENABLE_PYTHON "Enable Python payload support" ON)
option(ENABLE_NIM "Enable Nim payload support" ON)
option(ENABLE_RUST "Enable Rust payload support" ON)

# Check for required dependencies
if(ENABLE_PYTHON)
  find_package(Python3 COMPONENTS Interpreter QUIET)
  if(NOT Python3_FOUND)
    message(WARNING "Python 3 not found. Python payloads will be disabled.")
    set(ENABLE_PYTHON OFF)
  endif()
endif()

if(ENABLE_NIM)
  execute_process(
    COMMAND nim --version
    RESULT_VARIABLE NIM_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
  )
  if(NOT ${NIM_RESULT} EQUAL 0)
    message(WARNING "Nim compiler not found. Nim payloads will be disabled.")
    set(ENABLE_NIM OFF)
  endif()
endif()

if(ENABLE_RUST)
  execute_process(
    COMMAND rustc --version
    RESULT_VARIABLE RUST_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
  )
  if(NOT ${RUST_RESULT} EQUAL 0)
    message(WARNING "Rust compiler not found. Rust payloads will be disabled.")
    set(ENABLE_RUST OFF)
  endif()
endif()

# Include external toolchain files
if(BUILD_CH569)
  set(CH569_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/firmware/cmake/ch569_toolchain.cmake"
      CACHE FILEPATH "Toolchain file for CH569")
endif()

if(BUILD_ESP32C6)
  set(ESP32C6_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/firmware/cmake/esp32c6_toolchain.cmake"
      CACHE FILEPATH "Toolchain file for ESP32-C6")
endif()

# Include path for our custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Common settings for all subprojects
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Function to create subproject that maintains separate build folder structure
function(add_subproject_target)
  # Parse arguments
  set(options "")
  set(oneValueArgs NAME TOOLCHAIN_FILE)
  set(multiValueArgs DEPENDS EXTRA_CMAKE_ARGS)
  cmake_parse_arguments(SUBPROJ "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  
  # Define build directory for this subproject
  set(SUBPROJ_BINARY_DIR "${CMAKE_BINARY_DIR}/${SUBPROJ_NAME}")
  
  # Create a list of dependency targets
  set(DEPENDS_TARGETS "")
  foreach(dep ${SUBPROJ_DEPENDS})
    list(APPEND DEPENDS_TARGETS "${dep}")
  endforeach()
  
  # Define ExternalProject_Add parameters
  set(SUBPROJ_ARGS
    DEPENDS ${DEPENDS_TARGETS}
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/firmware/${SUBPROJ_NAME}"
    BINARY_DIR "${SUBPROJ_BINARY_DIR}"
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
      $<$<BOOL:${SUBPROJ_TOOLCHAIN_FILE}>:-DCMAKE_TOOLCHAIN_FILE=${SUBPROJ_TOOLCHAIN_FILE}>
      -DPARENT_BUILD=ON
      -DPROJECT_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/firmware
      -DBUILD_TESTING=${BUILD_TESTING}
      -DENABLE_PYTHON=${ENABLE_PYTHON}
      -DENABLE_NIM=${ENABLE_NIM}
      -DENABLE_RUST=${ENABLE_RUST}
      ${SUBPROJ_EXTRA_CMAKE_ARGS}
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
  )
  
  # Add the external project
  include(ExternalProject)
  ExternalProject_Add(${SUBPROJ_NAME} ${SUBPROJ_ARGS})
  
  # Create build target that builds just this subproject
  add_custom_target(build_${SUBPROJ_NAME}
    COMMAND ${CMAKE_COMMAND} --build ${SUBPROJ_BINARY_DIR}
    COMMENT "Building ${SUBPROJ_NAME}"
  )
  
  # Create clean target that cleans just this subproject
  add_custom_target(clean_${SUBPROJ_NAME}
    COMMAND ${CMAKE_COMMAND} --build ${SUBPROJ_BINARY_DIR} --target clean
    COMMENT "Cleaning ${SUBPROJ_NAME}"
  )
  
  # Create test target if testing is enabled
  if(BUILD_TESTING)
    add_custom_target(test_${SUBPROJ_NAME}
      COMMAND ${CMAKE_COMMAND} --build ${SUBPROJ_BINARY_DIR} --target test
      COMMENT "Testing ${SUBPROJ_NAME}"
    )
  endif()
endfunction()

# Add "common" module (shared code, must be built first)
add_subproject_target(
  NAME common
  DEPENDS ""
)

# Add CH569 firmware module (depends on common)
if(BUILD_CH569)
  add_subproject_target(
    NAME ch569
    TOOLCHAIN_FILE ${CH569_TOOLCHAIN_FILE}
    DEPENDS common
  )
endif()

# Add ESP32-C6 firmware module (depends on common)
if(BUILD_ESP32C6)
  add_subproject_target(
    NAME esp32c6
    TOOLCHAIN_FILE ${ESP32C6_TOOLCHAIN_FILE}
    DEPENDS common
    EXTRA_CMAKE_ARGS
      -DIDF_PATH=$ENV{IDF_PATH}
  )
endif()

# Add tools module (native build for host machine)
if(BUILD_TOOLS)
  add_subproject_target(
    NAME tools
    DEPENDS ""
    EXTRA_CMAKE_ARGS
      -DENABLE_PYTHON=${ENABLE_PYTHON}
      -DENABLE_NIM=${ENABLE_NIM}
      -DENABLE_RUST=${ENABLE_RUST}
  )
endif()

# Add examples module (only if both firmware targets are being built)
if(BUILD_EXAMPLES AND BUILD_CH569 AND BUILD_ESP32C6)
  add_subproject_target(
    NAME examples
    DEPENDS ch569 esp32c6
    EXTRA_CMAKE_ARGS
      -DENABLE_PYTHON=${ENABLE_PYTHON}
      -DENABLE_NIM=${ENABLE_NIM}
      -DENABLE_RUST=${ENABLE_RUST}
  )
endif()

# Add documentation module
if(BUILD_DOCS)
  find_package(Doxygen QUIET)
  if(DOXYGEN_FOUND)
    add_subproject_target(
      NAME docs
      DEPENDS ""
    )
  else()
    message(WARNING "Doxygen not found. Documentation will not be built.")
    set(BUILD_DOCS OFF)
  endif()
endif()

# Add custom target to flash both controllers
if(BUILD_CH569 AND BUILD_ESP32C6)
  add_custom_target(flash
    COMMAND ${CMAKE_COMMAND} -E echo "Flashing CH569..."
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/ch569 --target flash_ch569
    COMMAND ${CMAKE_COMMAND} -E echo "Flashing ESP32-C6..."
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/esp32c6 --target flash_esp32c6
    COMMENT "Flashing both controllers"
  )
endif()

# Add complete test target if testing is enabled
if(BUILD_TESTING)
  add_custom_target(test_all)
  
  if(BUILD_CH569)
    add_dependencies(test_all test_ch569)
  endif()
  
  if(BUILD_ESP32C6)
    add_dependencies(test_all test_esp32c6)
  endif()
  
  add_dependencies(test_all test_common)
  
  if(BUILD_TOOLS)
    add_dependencies(test_all test_tools)
  endif()
endif()

# Add target to build all examples
if(BUILD_EXAMPLES AND BUILD_CH569 AND BUILD_ESP32C6)
  add_custom_target(build_all_examples
    DEPENDS build_examples
    COMMENT "Building all example payloads"
  )
endif()

# Add target to package release
add_custom_target(package
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/release
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/ch569 ${CMAKE_BINARY_DIR}/release/ch569
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/esp32c6 ${CMAKE_BINARY_DIR}/release/esp32c6
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/examples ${CMAKE_BINARY_DIR}/release/examples
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_BINARY_DIR}/release/
  COMMAND ${CMAKE_COMMAND} -E tar "cfz" "${CMAKE_BINARY_DIR}/usb-goose-${PROJECT_VERSION}.tar.gz" "${CMAKE_BINARY_DIR}/release"
  COMMENT "Creating release package"
)

# Print build configuration
message(STATUS "=====================================")
message(STATUS "USB Goose Build Configuration:")
message(STATUS "=====================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Building CH569 firmware: ${BUILD_CH569}")
message(STATUS "Building ESP32-C6 firmware: ${BUILD_ESP32C6}")
message(STATUS "Building tools: ${BUILD_TOOLS}")
message(STATUS "Building examples: ${BUILD_EXAMPLES}")
message(STATUS "Building docs: ${BUILD_DOCS}")
message(STATUS "Building tests: ${BUILD_TESTING}")
message(STATUS "Python support: ${ENABLE_PYTHON}")
message(STATUS "Nim support: ${ENABLE_NIM}")
message(STATUS "Rust support: ${ENABLE_RUST}")
message(STATUS "=====================================")