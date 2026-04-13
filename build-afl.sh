#!/bin/bash
# AFL++ instrumented build script for PocketSphinx

set -e

# Check if AFL++ is installed
if ! command -v afl-clang-fast &> /dev/null; then
    echo "Error: AFL++ not found. Install with: sudo apt install afl++"
    exit 1
fi

# Set AFL++ compiler flags
export CC=afl-clang-fast
export CXX=afl-clang-fast++
export AFL_USE_ASAN=1  # Enable AddressSanitizer for better bug detection
export AFL_USE_UBSAN=1  # Enable UndefinedBehaviorSanitizer

# Create build directory
BUILD_DIR="build-afl"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_FUZZING=ON \
      ..

# Build
make -j$(nproc)

echo ""
echo "============================================"
echo "AFL++ instrumented build complete!"
echo "============================================"
echo "Build directory: $BUILD_DIR"
echo ""
echo "Built fuzzers:"
ls -lh fuzz_* 2>/dev/null || echo "  (no fuzzers found)"
echo ""
echo "Next steps:"
echo "  1. cd fuzzing/"
echo "  2. ./run_fuzzer.sh jsgf"
echo ""
