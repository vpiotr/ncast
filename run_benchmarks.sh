#!/bin/bash
# run_benchmarks.sh - Execute all benchmarks and verify exit status

set -e

echo "=== ncast benchmark execution script ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Running rebuild first..."
    ./rebuild.sh
fi

cd build

echo "Running benchmarks..."

# Find all benchmark executables
benchmark_count=0
success_count=0

for benchmark in benchmark_*; do
    if [ -x "$benchmark" ]; then
        benchmark_count=$((benchmark_count + 1))
        benchmark_name=$(basename "$benchmark")
        echo "Running $benchmark_name..."
        
        if "./$benchmark"; then
            echo "✓ $benchmark_name completed successfully"
            success_count=$((success_count + 1))
        else
            echo "✗ $benchmark_name failed with exit code $?"
        fi
        echo ""
    fi
done

if [ $benchmark_count -eq 0 ]; then
    echo "No benchmark executables found"
    exit 1
fi

echo "=== Benchmark Results ==="
echo "Total benchmarks: $benchmark_count"
echo "Successful: $success_count"
echo "Failed: $((benchmark_count - success_count))"

if [ $success_count -eq $benchmark_count ]; then
    echo "All benchmarks completed successfully!"
    exit 0
else
    echo "Some benchmarks failed!"
    exit 1
fi
