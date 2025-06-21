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

# Check if Doxyfile exists
if [ ! -f "docs/Doxyfile" ]; then
    echo "Error: Doxyfile not found in docs/ directory"
    echo "Please run cmake to generate the Doxyfile first"
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
