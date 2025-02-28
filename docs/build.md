# USB Goose: Build Instructions

This document provides detailed instructions for building the USB Goose firmware and payloads.

## Prerequisites

### Required Tools

* **CMake** (version 3.16 or higher)
* **LLVM/Clang** (recent version with RISC-V target support)
* **Python 3** (for build scripts and Python payloads)
* **Git** (for version control)

### Optional Tools for Additional Language Support

* **Pythran** (for Python payloads): `pip install pythran`
* **Nim** (for Nim payloads): [Installation Instructions](https://nim-lang.org/install.html)
* **Rust** (for Rust payloads): [Installation Instructions](https://www.rust-lang.org/tools/install)

### Environment Setup

1. **Set up RISC-V LLVM toolchain**

   The build system requires a RISC-V capable LLVM toolchain for both CH569 and ESP32-C6 targets.

   Set the `RISCV_LLVM_ROOT` environment variable to point to your LLVM installation:

   ```bash
   export RISCV_LLVM_ROOT=/path/to/your/llvm
   ```

2. **ESP-IDF setup** (required for ESP32-C6)

   If building the ESP32-C6 firmware:

   ```bash
   # Set up ESP-IDF environment
   export IDF_PATH=/path/to/esp-idf
   source $IDF_PATH/export.sh
   ```

3. **Rust Setup** (if using Rust payloads)

   ```bash
   # Install required components
   rustup component add rust-src
   rustup target add riscv32imac-unknown-none-elf
   ```

## Building the Project

### Basic Build

```bash
# Clone the repository
git clone https://github.com/yourusername/usb-goose.git
cd usb-goose

# Create build directory
mkdir build && cd build

# Configure the build
cmake ..

# Build everything
make
```

### Build Options

The build system supports several options to customize the build:

```bash
# Configure with specific options
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_CH569=ON \
  -DBUILD_ESP32C6=ON \
  -DBUILD_TOOLS=ON \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_DOCS=ON \
  -DENABLE_PYTHON=ON \
  -DENABLE_NIM=ON \
  -DENABLE_RUST=ON
```

Available options:

* `CMAKE_BUILD_TYPE`: Set to `Debug` (default), `Release`, or `RelWithDebInfo`
* `BUILD_CH569`: Enable/disable CH569 firmware build (ON by default)
* `BUILD_ESP32C6`: Enable/disable ESP32-C6 firmware build (ON by default)
* `BUILD_TOOLS`: Enable/disable development tools build (ON by default)
* `BUILD_EXAMPLES`: Enable/disable example payloads build (ON by default)
* `BUILD_DOCS`: Enable/disable documentation build (ON by default)
* `BUILD_TESTING`: Enable/disable test build (ON by default)
* `ENABLE_PYTHON`: Enable/disable Python payload support (ON by default)
* `ENABLE_NIM`: Enable/disable Nim payload support (ON by default)
* `ENABLE_RUST`: Enable/disable Rust payload support (ON by default)

### Building Specific Components

You can build specific components of the project:

```bash
# Build only the common library
make build_common

# Build only the CH569 firmware
make build_ch569

# Build only the ESP32-C6 firmware
make build_esp32c6

# Build only the tools
make build_tools

# Build only the examples
make build_examples

# Build only the documentation
make build_docs
```

## Flashing the Firmware

### Flashing CH569

```bash
# Flash CH569 firmware
make flash_ch569
```

By default, this will use `/dev/ttyUSB0` on Linux/macOS or `COM3` on Windows. 
You can specify a different port:

```bash
cmake .. -DCH569_PORT=/dev/ttyACM0
make flash_ch569
```

### Flashing ESP32-C6

```bash
# Flash ESP32-C6 firmware
make flash_esp32c6
```

By default, this will use `/dev/ttyUSB1` on Linux/macOS or `COM4` on Windows.
You can specify a different port:

```bash
cmake .. -DESP32C6_PORT=/dev/ttyACM1
make flash_esp32c6
```

### Flashing Both Controllers

```bash
# Flash both controllers at once
make flash
```

## Working with Payloads

### Building Example Payloads

Example payloads will be built automatically if `BUILD_EXAMPLES` is enabled:

```bash
# Explicitly build all examples
make build_all_examples
```

The compiled payloads will be placed in:
- `build/ch569/payloads/` for CH569 payloads
- `build/esp32c6/payloads/` for ESP32-C6 payloads

### Creating New Payloads

#### C Payloads

Create a new `.c` file in the appropriate directory:
- `firmware/examples/usb_payloads/` for USB-only payloads
- `firmware/examples/wireless_payloads/` for wireless-only payloads
- `firmware/examples/combined_payloads/` for dual-chip payloads

Add the new file to `firmware/examples/CMakeLists.txt` by adding a new `add_c_payload()` entry.

#### C++ Payloads

Similar to C payloads, but use `add_cpp_payload()` in the CMakeLists.txt file.

#### Python Payloads

Create a new `.py` file in `firmware/examples/python_payloads/` and add a new `add_python_payload()` entry to the CMakeLists.txt file.

Python payloads must include the Pythran export line:

```python
#pythran export payload_main(int, int, int, str, str, int)
```

#### Nim Payloads

Create a new `.nim` file in `firmware/examples/nim_payloads/` and add a new `add_nim_payload()` entry to the CMakeLists.txt file.

#### Rust Payloads

Create a new `.rs` file in `firmware/examples/rust_payloads/` directory and add appropriate build commands in the CMakeLists.txt file.

## Troubleshooting

### Common Issues

1. **LLVM/Clang not found**
   
   Make sure you have LLVM installed with RISC-V target support and the `RISCV_LLVM_ROOT` environment variable is set correctly.

2. **ESP-IDF not found**
   
   Ensure that the `IDF_PATH` environment variable is set and you've sourced the ESP-IDF export script.

3. **Build errors with Pythran**
   
   Make sure you have Pythran installed: `pip install pythran`

4. **Build errors with Nim**
   
   Make sure you have Nim installed and in your PATH.

5. **Build errors with Rust**
   
   Ensure you have Rust installed with the rust-src component and RISC-V target.

### Getting Help

If you encounter issues not covered here, please:

1. Check the detailed logs in the build directory
2. Open an issue on the project's GitHub repository
3. Join our community forum for assistance

## Advanced Topics

### Custom Toolchains

You can use custom toolchain files by setting the toolchain file path:

```bash
cmake .. -DCH569_TOOLCHAIN_FILE=/path/to/your/toolchain.cmake -DESP32C6_TOOLCHAIN_FILE=/path/to/your/esp_toolchain.cmake
```

### Creating a Release Package

```bash
# Create a release package with all built binaries
make package
```

This will create a `usb-goose-x.y.z.tar.gz` file in the build directory with all the necessary files.

### Enabling Verbose Build Output

For debugging build issues:

```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
```

## License

This project is licensed under [LICENSE INFORMATION].