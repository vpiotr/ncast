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
    # Fallback to direct execution of modular test suite
    echo "ctest not available, running modular tests directly..."
    echo ""
    
    tests_passed=0
    tests_total=0
    
    # List of test modules
    test_modules=("test_ncast_core" "test_ncast_int" "test_ncast_float" "test_ncast_char")
    
    for test in "${test_modules[@]}"; do
        if [ -f "./$test" ]; then
            echo "=== Running $test ==="
            if "./$test"; then
                echo "✓ $test passed"
                ((tests_passed++))
            else
                echo "✗ $test failed"
            fi
            ((tests_total++))
            echo ""
        else
            echo "Warning: $test executable not found"
        fi
    done
    
    echo "=== Test Summary ==="
    echo "Passed: $tests_passed/$tests_total test modules"
    
    if [ $tests_passed -ne $tests_total ]; then
        echo "Some tests failed!"
        exit 1
    fi
fi

echo "All tests completed successfully!"
