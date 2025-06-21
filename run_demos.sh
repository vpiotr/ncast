#!/bin/bash
# run_demos.sh - Execute all demos and verify exit status

set -e

echo "=== ncast demo execution script ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Running rebuild first..."
    ./rebuild.sh
fi

cd build

echo "Running demos..."

# Find all demo executables
demo_count=0
success_count=0

# Check for demo_* executables only
for demo in demo_*; do
    if [ -x "$demo" ]; then
        demo_count=$((demo_count + 1))
        demo_name=$(basename "$demo")
        echo "Running $demo_name..."
        
        if "./$demo"; then
            echo "✓ $demo_name completed successfully"
            success_count=$((success_count + 1))
        else
            echo "✗ $demo_name failed with exit code $?"
        fi
        echo ""
    fi
done

if [ $demo_count -eq 0 ]; then
    echo "No demo executables found"
    exit 1
fi

echo "=== Demo Results ==="
echo "Total demos: $demo_count"
echo "Successful: $success_count"
echo "Failed: $((demo_count - success_count))"

if [ $success_count -eq $demo_count ]; then
    echo "All demos completed successfully!"
    exit 0
else
    echo "Some demos failed!"
    exit 1
fi
