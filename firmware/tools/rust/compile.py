#!/usr/bin/env python3
"""
compile.py - Rust compiler wrapper for USB Goose payloads

This script compiles Rust payloads to LLVM IR and then uses
Clang to generate target-specific ELF files.
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

def ensure_rust():
    """Check if Rust is installed, give instructions if not."""
    try:
        subprocess.check_call(["rustc", "--version"], stdout=subprocess.DEVNULL)
        return True
    except (subprocess.SubprocessError, FileNotFoundError):
        print("Rust compiler not found. Please install Rust:")
        print("  - Visit https://www.rust-lang.org/tools/install")
        print("  - Or run: curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh")
        return False

def check_rust_components():
    """Check if necessary Rust components are installed."""
    try:
        # Check for rust-src component
        result = subprocess.run(
            ["rustup", "component", "list", "--installed"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        if "rust-src" not in result.stdout:
            print("Adding rust-src component...")
            subprocess.check_call(["rustup", "component", "add", "rust-src"])
        
        return True
    except subprocess.SubprocessError:
        print("Failed to check Rust components. Make sure rustup is installed.")
        return False

def create_cargo_config(target_dir, target_triple):
    """Create a Cargo config file for cross-compilation."""
    os.makedirs(os.path.join(target_dir, ".cargo"), exist_ok=True)
    
    config_path = os.path.join(target_dir, ".cargo", "config.toml")
    with open(config_path, 'w') as f:
        f.write(f"""[build]
target = "{target_triple}"

[unstable]
build-std = ["core", "alloc"]
build-std-features = ["compiler-builtins-mem"]

[target.{target_triple}]
rustflags = [
    "-C", "linker=clang",
    "-C", "link-arg=-nostartfiles",
    "-C", "link-arg=-nostdlib",
    "-C", "link-arg=-static",
    "-C", "opt-level=s",
]
""")
    
    return config_path

def compile_rust_to_ir(rust_file, output_dir, target):
    """Compile Rust to LLVM IR."""
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Get target settings
    target_settings = TARGETS[target]
    target_triple = target_settings["triple"]
    
    # Create a Cargo.toml file
    basename = os.path.basename(rust_file)
    module_name = os.path.splitext(basename)[0]
    cargo_toml = os.path.join(output_dir, "Cargo.toml")
    
    with open(cargo_toml, 'w') as f:
        f.write(f"""[package]
name = "{module_name}"
version = "0.1.0"
edition = "2021"

[lib]
name = "{module_name}"
path = "{os.path.abspath(rust_file)}"
crate-type = ["staticlib"]

[profile.dev]
opt-level = 's'
debug = true
panic = "abort"

[profile.release]
opt-level = 's'
lto = true
debug = false
panic = "abort"
""")
    
    # Create Cargo config file
    create_cargo_config(output_dir, target_triple)
    
    # Create src directory with bindings
    src_dir = os.path.join(output_dir, "src")
    os.makedirs(src_dir, exist_ok=True)
    
    # Create a main.rs that includes the source file
    main_path = os.path.join(src_dir, "lib.rs")
    with open(main_path, 'w') as f:
        f.write(f'#![no_std]\n#![no_main]\n\ninclude!("{os.path.abspath(rust_file)}");')
    
    # Compile using Cargo
    env = os.environ.copy()
    env["RUSTFLAGS"] = f"-C linker=clang -C target-cpu={target_settings['cpu']}"
    
    print(f"Building Rust project in {output_dir}...")
    
    subprocess.check_call(
        ["cargo", "build", "--release"],
        cwd=output_dir,
        env=env
    )
    
    # Return the path to the compiled library
    return os.path.join(output_dir, "target", target_triple, "release", f"lib{module_name}.a")

def compile_object_to_elf(object_file, target, output_file=None):
    """Compile object file to ELF using LLVM/Clang."""
    # Get target settings
    target_settings = TARGETS[target]
    
    # Set output filename if not specified
    if output_file is None:
        basename = os.path.basename(object_file)
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
        "clang",
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
        object_file,
        "-Wl,--gc-sections",
    ]
    
    # Execute clang
    print(f"Compiling object to ELF for {target}...")
    print(f"Command: {' '.join(clang_cmd)}")
    subprocess.check_call(clang_cmd)
    
    return output_file

def main():
    parser = argparse.ArgumentParser(description="Compile Rust payloads to ELF files")
    parser.add_argument("rust_file", help="Rust file to compile")
    parser.add_argument("--target", choices=TARGETS.keys(), default="ch569", help="Target chip")
    parser.add_argument("--output", "-o", help="Output ELF file")
    parser.add_argument("--keep-ir", action="store_true", help="Keep generated LLVM IR file")
    parser.add_argument("--test", action="store_true", help="Run a simple test to verify the compiler works")
    args = parser.parse_args()

    if args.test:
        print("Running compiler test...")
        # Create a simple Rust test file
        with tempfile.NamedTemporaryFile(suffix='.rs', mode='w', delete=False) as f:
            f.write("""
#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[no_mangle]
pub extern "C" fn payload_main() -> i32 {
    0
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
""")
            test_file = f.name
        
        try:
            # Try to compile it
            with tempfile.TemporaryDirectory() as temp_dir:
                object_file = compile_rust_to_ir(test_file, temp_dir, "ch569")
                if os.path.exists(object_file):
                    print("Compiler test successful!")
                    return 0
                else:
                    print("Compiler test failed: object file not created")
                    return 1
        finally:
            os.unlink(test_file)
        
    # Ensure Rust is installed
    if not ensure_rust():
        return 1
    
    # Check for required Rust components
    if not check_rust_components():
        return 1
    
    # Create temporary directory
    with tempfile.TemporaryDirectory() as temp_dir:
        # Compile Rust to object file
        object_file = compile_rust_to_ir(args.rust_file, temp_dir, args.target)
        
        # Compile object file to ELF
        elf_file = compile_object_to_elf(object_file, args.target, args.output)
        
        # Copy resulting ELF file if output directory is different
        if args.output and os.path.dirname(args.output) != temp_dir:
            os.makedirs(os.path.dirname(args.output), exist_ok=True)
            shutil.copy2(elf_file, args.output)
        
        # Keep IR file if requested
        if args.keep_ir:
            ir_dest = os.path.splitext(elf_file)[0] + ".bc"
            shutil.copy2(object_file, ir_dest)
            print(f"IR file saved to: {ir_dest}")
    
    print(f"Compiled ELF file: {elf_file}")
    return 0

if __name__ == "__main__":
    sys.exit(main())