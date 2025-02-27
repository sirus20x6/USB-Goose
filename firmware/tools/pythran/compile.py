#!/usr/bin/env python3
"""
compile.py - Pythran compiler wrapper for USB Goose payloads

This script compiles Python payloads to C/C++ using Pythran and then 
uses LLVM/Clang to generate target-specific ELF files.
"""

import os
import sys
import argparse
import subprocess
import tempfile
import shutil
from pathlib import Path

# Define target-specific settings
TARGETS = {
    "ch569": {
        "triple": "riscv32-unknown-none-elf",
        "cpu": "rv32imac",
        "abi": "ilp32",
        "defines": ["CH569", "RISCV", "USE_USB3"],
        "include_dirs": ["../../firmware/common/include", "../../firmware/ch569/include"],
        "linker_script": "../../firmware/ch569/ch569_payload.ld"
    },
    "esp32c6": {
        "triple": "riscv32-unknown-none-elf",
        "cpu": "rv32imc",
        "abi": "ilp32",
        "defines": ["ESP32C6", "ESP_PLATFORM"],
        "include_dirs": ["../../firmware/common/include", "../../firmware/esp32c6/include"],
        "linker_script": "../../firmware/esp32c6/esp32c6_payload.ld"
    }
}

def ensure_pythran():
    """Check if Pythran is installed, install if not."""
    try:
        import pythran
        return True
    except ImportError:
        print("Pythran not found. Installing...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", "pythran"])
        return True

def compile_python_to_cpp(python_file, output_dir):
    """Compile Python to C++ using Pythran."""
    import pythran
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate output C++ filename
    basename = os.path.basename(python_file)
    cpp_file = os.path.join(output_dir, os.path.splitext(basename)[0] + ".cpp")
    
    # Read Python source
    with open(python_file, 'r') as f:
        python_source = f.read()
    
    # Get pythran config
    config = pythran.frontend.get_config(python_source)
    
    # Generate C++ code
    cxx = pythran.compile(python_source, config)
    
    # Add framework bindings
    bindings_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "bindings.cpp")
    with open(bindings_path, 'r') as f:
        bindings = f.read()
    
    # Write combined C++ file
    with open(cpp_file, 'w') as f:
        f.write(cxx)
        f.write("\n\n// Framework bindings\n")
        f.write(bindings)
    
    return cpp_file

def compile_cpp_to_elf(cpp_file, target, output_file=None):
    """Compile C++ to ELF using LLVM/Clang."""
    # Get target settings
    target_settings = TARGETS[target]
    
    # Set output filename if not specified
    if output_file is None:
        basename = os.path.basename(cpp_file)
        output_file = os.path.splitext(basename)[0] + ".elf"
    
    # Build include directory arguments
    include_args = []
    for include_dir in target_settings["include_dirs"]:
        include_args.extend(["-I", include_dir])
    
    # Build define arguments
    define_args = []
    for define in target_settings["defines"]:
        define_args.append(f"-D{define}")
    
    # Build clang command
    clang_cmd = [
        "clang++",
        "-target", target_settings["triple"],
        f"-mcpu={target_settings['cpu']}",
        f"-mabi={target_settings['abi']}",
        "-O2",
        "-ffunction-sections",
        "-fdata-sections",
        "-nostdlib",
        "-ffreestanding",
        *include_args,
        *define_args,
        "-T", target_settings["linker_script"],
        "-o", output_file,
        cpp_file,
        "-Wl,--gc-sections",
    ]
    
    # Execute clang
    print(f"Compiling {cpp_file} for {target}...")
    subprocess.check_call(clang_cmd)
    
    return output_file

def main():
    parser = argparse.ArgumentParser(description="Compile Python payloads to ELF files")
    parser.add_argument("python_file", help="Python file to compile")
    parser.add_argument("--target", choices=TARGETS.keys(), default="ch569", help="Target chip")
    parser.add_argument("--output", "-o", help="Output ELF file")
    parser.add_argument("--keep-cpp", action="store_true", help="Keep generated C++ file")
    args = parser.parse_args()
    
    # Ensure Pythran is installed
    if not ensure_pythran():
        print("Failed to install Pythran. Please install manually: pip install pythran")
        return 1
    
    # Create temporary directory
    with tempfile.TemporaryDirectory() as temp_dir:
        # Compile Python to C++
        cpp_file = compile_python_to_cpp(args.python_file, temp_dir)
        
        # Compile C++ to ELF
        elf_file = compile_cpp_to_elf(cpp_file, args.target, args.output)
        
        # Copy resulting ELF file if output directory is different
        if args.output and os.path.dirname(args.output) != temp_dir:
            os.makedirs(os.path.dirname(args.output), exist_ok=True)
            shutil.copy2(os.path.join(temp_dir, os.path.basename(elf_file)), args.output)
        
        # Copy C++ file if requested
        if args.keep_cpp:
            cpp_dest = os.path.splitext(elf_file)[0] + ".cpp"
            shutil.copy2(cpp_file, cpp_dest)
            print(f"C++ file saved to: {cpp_dest}")
    
    print(f"Compiled ELF file: {elf_file}")
    return 0

if __name__ == "__main__":
    sys.exit(main())