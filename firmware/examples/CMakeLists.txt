cmake_minimum_required(VERSION 3.16)
project(USBGooseExamples VERSION 0.2.0 LANGUAGES C CXX)

# Initialize list of example targets
set(EXAMPLE_TARGETS "")

# Find common libraries
if(DEFINED PROJECT_ROOT)
    # When building from superbuild, include from project root
    set(COMMON_DIR "${PROJECT_ROOT}/common")
    include_directories(${COMMON_DIR}/include)
else()
    # When building standalone, use find_package
    find_package(DuckyCommon REQUIRED)
endif()

# Add C example payloads
function(add_c_payload NAME SOURCES TARGET_CHIPS)
    foreach(CHIP ${TARGET_CHIPS})
        set(TARGET_NAME "${NAME}_${CHIP}")
        add_executable(${TARGET_NAME} ${SOURCES})
        
        if(DEFINED PROJECT_ROOT)
            target_include_directories(${TARGET_NAME} PRIVATE 
                ${PROJECT_ROOT}/common/include
                ${PROJECT_ROOT}/${CHIP}/include
            )
            target_link_libraries(${TARGET_NAME} PRIVATE 
                "${PROJECT_ROOT}/common/build/libducky_common.a"
            )
        else()
            target_link_libraries(${TARGET_NAME} PRIVATE DuckyCommon::ducky_common)
        endif()
        
        # Set output directory based on chip
        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CHIP}/payloads"
        )
        
        # Add to targets list
        list(APPEND EXAMPLE_TARGETS ${TARGET_NAME})
    endforeach()
    
    # Update the parent scope variable
    set(EXAMPLE_TARGETS ${EXAMPLE_TARGETS} PARENT_SCOPE)
endfunction()

# Add C++ example payloads
function(add_cpp_payload NAME SOURCES TARGET_CHIPS)
    foreach(CHIP ${TARGET_CHIPS})
        set(TARGET_NAME "${NAME}_${CHIP}")
        add_executable(${TARGET_NAME} ${SOURCES})
        
        if(DEFINED PROJECT_ROOT)
            target_include_directories(${TARGET_NAME} PRIVATE 
                ${PROJECT_ROOT}/common/include
                ${PROJECT_ROOT}/${CHIP}/include
            )
            target_link_libraries(${TARGET_NAME} PRIVATE 
                "${PROJECT_ROOT}/common/build/libducky_common.a"
            )
        else()
            target_link_libraries(${TARGET_NAME} PRIVATE 
                DuckyCommon::ducky_common
                DuckyCommon::ducky_common_cpp
            )
        endif()
        
        # Set C++ standard
        set_target_properties(${TARGET_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CHIP}/payloads"
        )
        
        # Add to targets list
        list(APPEND EXAMPLE_TARGETS ${TARGET_NAME})
    endforeach()
    
    # Update the parent scope variable
    set(EXAMPLE_TARGETS ${EXAMPLE_TARGETS} PARENT_SCOPE)
endfunction()

# Set up chip list
set(ALL_CHIPS "ch569;esp32c6")
set(USB_CHIPS "ch569")
set(WIRELESS_CHIPS "esp32c6")

# Add USB payloads (CH569 only)
add_c_payload(os_detect_basic 
              "${CMAKE_CURRENT_SOURCE_DIR}/usb_payloads/os_detect_basic.c"
              "${USB_CHIPS}")

# Add wireless payloads (ESP32-C6 only)
if(ENABLE_RUST)
    # We'll handle Rust separately with custom commands
    set(RUST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/wireless_payloads/wifi_recon.rs")
    set(RUST_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/esp32c6/payloads/wifi_recon_esp32c6.elf")
    
    add_custom_command(
        OUTPUT ${RUST_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/esp32c6/payloads
        COMMAND ${PROJECT_ROOT}/tools/rust/compile.py
                ${RUST_SOURCES}
                --target esp32c6
                --output ${RUST_OUTPUT}
        DEPENDS ${RUST_SOURCES}
        COMMENT "Compiling Rust payload wifi_recon for ESP32-C6"
    )
    
    add_custom_target(wifi_recon_esp32c6 ALL DEPENDS ${RUST_OUTPUT})
    list(APPEND EXAMPLE_TARGETS wifi_recon_esp32c6)
endif()

# Add combined payloads (both chips)
add_c_payload(test_interchip
              "${CMAKE_CURRENT_SOURCE_DIR}/combined_payloads/test_interchip.c"
              "${ALL_CHIPS}")

add_cpp_payload(exfiltration
                "${CMAKE_CURRENT_SOURCE_DIR}/combined_payloads/exfiltration.cpp"
                "${ALL_CHIPS}")

add_c_payload(multi_vector_exfil
              "${CMAKE_CURRENT_SOURCE_DIR}/combined_payloads/multi_vector_exfil.c"
              "${ALL_CHIPS}")

# Add Python payloads if enabled
if(ENABLE_PYTHON)
    # Find Python interpreter
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    
    # Define a function to add Python payload targets
    function(add_python_payload NAME SOURCE TARGET_CHIPS)
        foreach(CHIP ${TARGET_CHIPS})
            set(TARGET_NAME "${NAME}_py_${CHIP}")
            set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CHIP}/payloads/${NAME}_${CHIP}.elf")
            set(PYTHRAN_COMPILER "${PROJECT_ROOT}/tools/pythran/compile.py")
            
            # Add custom command to compile Python to ELF
            add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${CHIP}/payloads
                COMMAND ${Python3_EXECUTABLE} ${PYTHRAN_COMPILER} 
                        ${SOURCE} 
                        --target ${CHIP} 
                        --output ${OUTPUT_FILE}
                DEPENDS ${SOURCE} ${PYTHRAN_COMPILER}
                COMMENT "Compiling Python payload ${NAME} for ${CHIP}"
            )
            
            # Add custom target
            add_custom_target(${TARGET_NAME} ALL
                DEPENDS ${OUTPUT_FILE}
            )
            
            # Add to the list of examples
            list(APPEND EXAMPLE_TARGETS ${TARGET_NAME})
        endforeach()
        
        # Update the parent scope variable
        set(EXAMPLE_TARGETS ${EXAMPLE_TARGETS} PARENT_SCOPE)
    endfunction()
    
    # Add Python payloads
    add_python_payload(os_detect
                      "${CMAKE_CURRENT_SOURCE_DIR}/python_payloads/os_detect.py"
                      "${ALL_CHIPS}")
else()
    message(STATUS "Python support disabled. Python payloads will not be built.")
endif()

# Add Nim payloads if enabled
if(ENABLE_NIM)
    # Define a function to add Nim payload targets
    function(add_nim_payload NAME SOURCE TARGET_CHIPS)
        foreach(CHIP ${TARGET_CHIPS})
            set(TARGET_NAME "${NAME}_nim_${CHIP}")
            set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CHIP}/payloads/${NAME}_${CHIP}.elf")
            set(NIM_COMPILER "${PROJECT_ROOT}/tools/nim/compile.py")
            
            # Add custom command to compile Nim to ELF
            add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${CHIP}/payloads
                COMMAND ${Python3_EXECUTABLE} ${NIM_COMPILER} 
                        ${SOURCE} 
                        --target ${CHIP} 
                        --output ${OUTPUT_FILE}
                DEPENDS ${SOURCE} ${NIM_COMPILER}
                COMMENT "Compiling Nim payload ${NAME} for ${CHIP}"
            )
            
            # Add custom target
            add_custom_target(${TARGET_NAME} ALL
                DEPENDS ${OUTPUT_FILE}
            )
            
            # Add to the list of examples
            list(APPEND EXAMPLE_TARGETS ${TARGET_NAME})
        endforeach()
        
        # Update the parent scope variable
        set(EXAMPLE_TARGETS ${EXAMPLE_TARGETS} PARENT_SCOPE)
    endfunction()
    
    # Add Nim payloads
    add_nim_payload(os_detect
                   "${CMAKE_CURRENT_SOURCE_DIR}/nim_payloads/os_detect.nim"
                   "${ALL_CHIPS}")
else()
    message(STATUS "Nim support disabled. Nim payloads will not be built.")
endif()

# Add a target to build all examples
add_custom_target(examples ALL DEPENDS ${EXAMPLE_TARGETS})

# Print summary
message(STATUS "Configured example payloads:")
foreach(TARGET ${EXAMPLE_TARGETS})
    message(STATUS "  - ${TARGET}")
endforeach()