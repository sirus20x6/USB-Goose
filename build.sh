#!/bin/bash
# build.sh - Build script for USB Goose project

# Stop on errors
set -e

# Default build configuration
BUILD_TYPE="Debug"
BUILD_CH569=ON
BUILD_ESP32C6=ON
BUILD_TOOLS=ON
BUILD_EXAMPLES=ON
BUILD_DOCS=ON
BUILD_TESTING=ON
CLEAN=false
VERBOSE=false

# Function to print help
print_help() {
    echo "USB Goose Build Script"
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -c, --clean             Clean build directory before building"
    echo "  -v, --verbose           Enable verbose CMake output"
    echo "  -t, --type <type>       Set build type (Debug, Release, RelWithDebInfo)"
    echo "  --no-ch569              Skip CH569 firmware build"
    echo "  --no-esp32c6            Skip ESP32-C6 firmware build"
    echo "  --no-tools              Skip tools build"
    echo "  --no-examples           Skip examples build"
    echo "  --no-docs               Skip documentation build"
    echo "  --no-tests              Skip tests build"
    echo ""
    echo "Targets:"
    echo "  all                     Build everything (default)"
    echo "  common                  Build only common library"
    echo "  ch569                   Build only CH569 firmware"
    echo "  esp32c6                 Build only ESP32-C6 firmware"
    echo "  tools                   Build only tools"
    echo "  examples                Build only examples"
    echo "  docs                    Build only documentation"
    echo "  flash                   Flash both controllers"
    echo "  flash_ch569             Flash only CH569"
    echo "  flash_esp32c6           Flash only ESP32-C6"
    echo "  test                    Run all tests"
    echo ""
}

# Parse command line arguments
TARGET="all"
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            print_help
            exit 0
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --no-ch569)
            BUILD_CH569=OFF
            shift
            ;;
        --no-esp32c6)
            BUILD_ESP32C6=OFF
            shift
            ;;
        --no-tools)
            BUILD_TOOLS=OFF
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES=OFF
            shift
            ;;
        --no-docs)
            BUILD_DOCS=OFF
            shift
            ;;
        --no-tests)
            BUILD_TESTING=OFF
            shift
            ;;
        common|ch569|esp32c6|tools|examples|docs|flash|flash_ch569|flash_esp32c6|test)
            TARGET="$1"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            print_help
            exit 1
            ;;
    esac
done

# Create build directory
BUILD_DIR="build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf *
fi

# Configure build options
CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_CH569=$BUILD_CH569"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_ESP32C6=$BUILD_ESP32C6"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_TOOLS=$BUILD_TOOLS"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_EXAMPLES=$BUILD_EXAMPLES"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_DOCS=$BUILD_DOCS"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_TESTING=$BUILD_TESTING"

# Enable verbose output if requested
if [ "$VERBOSE" = true ]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

# Run CMake
echo "Configuring with CMake..."
cmake $CMAKE_OPTIONS ..

# Build the requested target
echo "Building target: $TARGET"
case "$TARGET" in
    all)
        cmake --build .
        ;;
    common)
        cmake --build . --target build_common
        ;;
    ch569)
        cmake --build . --target build_ch569
        ;;
    esp32c6)
        cmake --build . --target build_esp32c6
        ;;
    tools)
        cmake --build . --target build_tools
        ;;
    examples)
        cmake --build . --target build_examples
        ;;
    docs)
        cmake --build . --target build_docs
        ;;
    flash)
        cmake --build . --target flash
        ;;
    flash_ch569)
        cmake --build . --target flash_ch569
        ;;
    flash_esp32c6)
        cmake --build . --target flash_esp32c6
        ;;
    test)
        cmake --build . --target test_all
        ;;
    *)
        echo "Unknown target: $TARGET"
        exit 1
        ;;
esac

echo "Build completed successfully!"