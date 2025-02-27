cmake_minimum_required(VERSION 3.16)
project(DuckyExamples VERSION 0.1.0 LANGUAGES C CXX)

# Find Python interpreter
find_package(Python3 COMPONENTS Interpreter QUIET)
set(PYTHON_AVAILABLE ${Python3_FOUND})

# Check for Pythran
find_package(Pythran QUIET)
set(PYTHRAN_AVAILABLE ${Pythran_FOUND})

# Check for Nim compiler
execute_process(
    COMMAND nim --version
    RESULT_VARIABLE NIM_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
)
set(NIM_AVAILABLE FALSE)
if(${NIM_RESULT} EQUAL 0)
    set(NIM_AVAILABLE TRUE)
endif()

# Initialize list of example targets
set(EXAMPLE_TARGETS "")

# Add C/C++ examples
# Add test_interchip example
add_executable(test_interchip ${CMAKE_CURRENT_SOURCE_DIR}/combined_payloads/test_interchip.c)
target_link_libraries(test_interchip PRIVATE ducky_common)
target_include_directories(test_interchip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common/include)
list(APPEND EXAMPLE_TARGETS test_interchip)

# Add OS detection example
add_executable(os_detect_basic ${CMAKE_CURRENT_SOURCE_DIR}/usb_payloads/os_detect_basic.c)
target_link_libraries(os_detect_basic PRIVATE ducky_common)
target_include_directories(os_detect_basic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common/include)
list(APPEND EXAMPLE_TARGETS os_detect_basic)

# Add exfiltration example
add_executable(exfiltration ${CMAKE_CURRENT_SOURCE_DIR}/combined_payloads/exfiltration.cpp)
target_link_libraries(exfiltration PRIVATE ducky_common)
target_include_directories(exfiltration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common/include)
list(APPEND EXAMPLE_TARGETS exfiltration)

# Add Python examples if Python is available
if(PYTHON_AVAILABLE)
    # Define a function to add Python payload targets
    function(add_python_payload NAME SOURCE TARGET_CHIP)
        set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_${TARGET_CHIP}.elf")
        set(PYTHRAN_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/../tools/pythran/compile.py")
        
        # Add custom command to compile Python to ELF
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${Python3_EXECUTABLE} ${PYTHRAN_COMPILER} 
                    ${SOURCE} 
                    --target ${TARGET_CHIP} 
                    --output ${OUTPUT_FILE}
            DEPENDS ${SOURCE} ${PYTHRAN_COMPILER}
            COMMENT "Compiling Python payload ${NAME} for ${TARGET_CHIP}"
        )
        
        # Add custom target
        add_custom_target(${NAME}_${TARGET_CHIP} ALL
            DEPENDS ${OUTPUT_FILE}
        )
        
        # Add to the list of examples
        list(APPEND EXAMPLE_TARGETS ${NAME}_${TARGET_CHIP})
        set(EXAMPLE_TARGETS ${EXAMPLE_TARGETS} PARENT_SCOPE)
    endfunction()
    
    # Add Python payloads for each target chip
    add_python_payload(os_detect_py 
                      ${CMAKE_CURRENT_SOURCE_DIR}/python_payloads/os_detect.py 
                      ch569)
    
    add_python_payload(os_detect_py 
                      ${CMAKE_CURRENT_SOURCE_DIR}/python_payloads/os_detect.py 
                      esp32c6)
                      
    # Additional Python payloads can be added here
else()
    message(STATUS "Python not found, skipping Python payloads")
endif()

# Add Nim examples if Nim is available
if(PYTHON_AVAILABLE AND NIM_AVAILABLE)
    # Define a function to add Nim payload targets
    function(add_nim_payload NAME SOURCE TARGET_CHIP)
        set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_${TARGET_CHIP}.elf")
        set(NIM_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/../tools/nim/compile.py")
        
        # Add custom command to compile Nim to ELF
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${Python3_EXECUTABLE} ${NIM_COMPILER} 
                    ${SOURCE} 
                    --target ${TARGET_CHIP} 
                    --output ${OUTPUT_FILE}
            DEPENDS ${SOURCE} ${NIM_COMPILER}
            COMMENT "Compiling Nim payload ${NAME} for ${TARGET_CHIP}"
        )
        
        # Add custom target
        add_custom_target(${NAME}_${TARGET_CHIP} ALL
            DEPENDS ${OUTPUT_FILE}
        )
        
        # Add to the list of examples
        list(APPEND EXAMPLE_TARGETS ${NAME}_${TARGET_CHIP})
        set(EXAMPLE_TARGETS ${EXAMPLE_TARGETS} PARENT_SCOPE)
    endfunction()
    
    # Add Nim payloads for each target chip
    add_nim_payload(os_detect_nim 
                   ${CMAKE_CURRENT_SOURCE_DIR}/nim_payloads/os_detect.nim 
                   ch569)
    
    add_nim_payload(os_detect_nim 
                   ${CMAKE_CURRENT_SOURCE_DIR}/nim_payloads/os_detect.nim 
                   esp32c6)
    
    # Additional Nim payloads can be added here
else()
    if(NOT NIM_AVAILABLE)
        message(STATUS "Nim compiler not found, skipping Nim payloads")
    endif()
endif()

# Add a target to build all examples
add_custom_target(examples ALL DEPENDS ${EXAMPLE_TARGETS})