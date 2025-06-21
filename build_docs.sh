#!/bin/bash
# build_docs.sh - Build Doxygen documentation

set -e

echo "=== ncast documentation build script ==="

# Check if Doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen is not installed. Please install doxygen to build documentation."
    exit 1
fi

# Ensure docs directory exists
mkdir -p docs

# Clean up any previous builds
rm -rf docs/html
rm -f docs/Doxyfile

# Always regenerate Doxyfile from Doxyfile.in
if [ -f "docs/Doxyfile.in" ]; then
    echo "Generating docs/Doxyfile from docs/Doxyfile.in..."
    # We specifically target only ncast.h and docs, excluding utest.h
    sed \
        -e 's/@PROJECT_NAME@/ncast/g' \
        -e 's/@PROJECT_VERSION@/1.0.0/g' \
        -e 's/@DOXYGEN_OUTPUT_DIR@/./g' \
        -e 's/@DOXYGEN_INPUT_DIR@/..\/include\/ncast ..\/docs/g' \
        -e 's/@CMAKE_SOURCE_DIR@/../g' \
        -e 's/@CMAKE_CURRENT_SOURCE_DIR@/./g' \
        -e 's/@CMAKE_CURRENT_BINARY_DIR@/./g' \
        docs/Doxyfile.in > docs/Doxyfile
    
    # Add EXCLUDE pattern to exclude utest.h
    echo "EXCLUDE                = ../include/utest" >> docs/Doxyfile
else
    echo "Error: docs/Doxyfile.in not found."
    exit 1
fi

echo "Building documentation with Doxygen..."
cd docs
doxygen Doxyfile

if [ -d "html" ]; then
    echo "Documentation successfully built in docs/html/"
    echo "Open docs/html/index.html in your browser to view the documentation"
else
    echo "Error: Documentation build failed"
    exit 1
fi
