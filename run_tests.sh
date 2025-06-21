#!/bin/bash
# run_tests.sh - Execute tests via cmake

set -e

echo "=== ncast test execution script ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Running rebuild first..."
    ./rebuild.sh
fi

cd build

echo "Running tests..."
if command -v ctest &> /dev/null; then
    # Use ctest if available
    ctest --output-on-failure
else
    # Fallback to direct execution
    echo "ctest not available, running tests directly..."
    if [ -f "./tests/test_ncast" ]; then
        echo "Running ncast tests..."
        ./tests/test_ncast
    else
        echo "Error: test_ncast executable not found"
        echo "Please build the project first with ./rebuild.sh"
        exit 1
    fi
fi

echo "All tests completed successfully!"
