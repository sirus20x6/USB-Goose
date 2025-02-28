cmake_minimum_required(VERSION 3.16)
project(USBGooseTools VERSION 0.2.0 LANGUAGES C CXX)

# Find Python interpreter for tool scripts
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Initialize list of tool targets
set(TOOL_TARGETS "")

# Add C/C++ executable tools
add_executable(test_comm_manager ${CMAKE_CURRENT_SOURCE_DIR}/test_comm_manager.c)
target_include_directories(test_comm_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common/include)
list(APPEND TOOL_TARGETS test_comm_manager)

# Install scripts to bin directory
install(PROGRAMS
    wch-flasher.py
    DESTINATION bin
)

# Python tools configuration
if(ENABLE_PYTHON)
    # Check for Pythran
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import pythran"
        RESULT_VARIABLE PYTHRAN_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT ${PYTHRAN_RESULT} EQUAL 0)
        message(WARNING "Pythran not found. Consider installing with 'pip install pythran'")
    endif()
    
    # Install Python compiler wrapper
    install(PROGRAMS
        pythran/compile.py
        DESTINATION bin
        RENAME pythran-compile.py
    )
    
    # Copy Python bindings
    install(FILES
        pythran/bindings.cpp
        DESTINATION share/usbgoose/pythran
    )
    
    # Create symlinks for easier access
    add_custom_target(python_tools_symlinks
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_INSTALL_PREFIX}/bin/pythran-compile.py
                ${CMAKE_BINARY_DIR}/pythran-compile
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_INSTALL_PREFIX}/share/usbgoose/pythran
                ${CMAKE_BINARY_DIR}/pythran-bindings
        COMMENT "Creating Python tools symlinks"
    )
    
    list(APPEND TOOL_TARGETS python_tools_symlinks)
endif()

# Nim tools configuration
if(ENABLE_NIM)
    # Check for Nim compiler
    execute_process(
        COMMAND nim --version
        RESULT_VARIABLE NIM_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT ${NIM_RESULT} EQUAL 0)
        message(WARNING "Nim compiler not found. Visit https://nim-lang.org/install.html for installation instructions")
    endif()
    
    # Install Nim compiler wrapper
    install(PROGRAMS
        nim/compile.py
        DESTINATION bin
        RENAME nim-compile.py
    )
    
    # Copy Nim bindings
    install(FILES
        nim/bindings.c
        DESTINATION share/usbgoose/nim
    )
    
    # Create symlinks for easier access
    add_custom_target(nim_tools_symlinks
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_INSTALL_PREFIX}/bin/nim-compile.py
                ${CMAKE_BINARY_DIR}/nim-compile
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_INSTALL_PREFIX}/share/usbgoose/nim
                ${CMAKE_BINARY_DIR}/nim-bindings
        COMMENT "Creating Nim tools symlinks"
    )
    
    list(APPEND TOOL_TARGETS nim_tools_symlinks)
endif()

# Rust tools configuration
if(ENABLE_RUST)
    # Check for Rust compiler
    execute_process(
        COMMAND rustc --version
        RESULT_VARIABLE RUST_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT ${RUST_RESULT} EQUAL 0)
        message(WARNING "Rust compiler not found. Visit https://www.rust-lang.org/tools/install for installation instructions")
    endif()
    
    # Create Rust compiler directory and wrapper script
    add_custom_target(rust_tools
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/rust
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/rust ${CMAKE_CURRENT_BINARY_DIR}/rust
        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_CURRENT_BINARY_DIR}/rust/compile.py
                ${CMAKE_BINARY_DIR}/rust-compile
        COMMENT "Setting up Rust tools"
    )
    
    list(APPEND TOOL_TARGETS rust_tools)
endif()

# Add a target to build all tools
add_custom_target(tools ALL DEPENDS ${TOOL_TARGETS})

# Add testing if enabled
if(BUILD_TESTING)
    enable_testing()
    
    # Test communication manager
    add_test(
        NAME test_comm_manager
        COMMAND test_comm_manager
    )
    
    # Add test for Pythran compiler if enabled
    if(ENABLE_PYTHON AND NOT ${PYTHRAN_RESULT} EQUAL 0)
        add_test(
            NAME test_pythran_compiler
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/pythran/compile.py --test
        )
    endif()
    
    # Add test for Nim compiler if enabled
    if(ENABLE_NIM AND NOT ${NIM_RESULT} EQUAL 0)
        add_test(
            NAME test_nim_compiler
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/nim/compile.py --test
        )
    endif()
endif()

# Print summary
message(STATUS "Configured tools:")
foreach(TARGET ${TOOL_TARGETS})
    message(STATUS "  - ${TARGET}")
endforeach()