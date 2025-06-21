#!/bin/bash
# rebuild.sh - Rebuild everything from scratch

set -e

echo "=== ncast rebuild script ==="
echo "Cleaning previous build..."

# Remove build directory if it exists
if [ -d "build" ]; then
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "Building..."
make -j$(nproc)

echo "Build completed successfully!"
echo ""
echo "Available targets:"
echo "  test_ncast  - Run unit tests"
echo "  demo_ncast  - Run demonstration"
echo "  docs        - Generate documentation (if Doxygen available)"
echo ""
echo "To run tests: cd build && make test"
echo "To run demo: cd build && ./demo_ncast"
