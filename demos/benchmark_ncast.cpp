/**
 * @file benchmark_ncast.cpp
 * @brief Performance benchmark for ncast library with statistical analysis
 * 
 * This benchmark compares five casting approaches:
 * 1. static_cast (baseline)
 * 2. numeric_cast with validation disabled (via separate module)
 * 3. numeric_cast with validation enabled
 * 4. NUMERIC_CAST macro with validation disabled (via separate module)
 * 5. NUMERIC_CAST macro with validation enabled
 * 
 * Features statistical analysis with multiple runs, calculating:
 * - Average, median, standard deviation, min, max for each approach
 * - Overhead percentages relative to static_cast baseline
 * 
 * Usage: ./benchmark_ncast [number_of_runs]
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include "../include/ncast/ncast.h"
#include "benchmark_ncast_no_validation.h"

using namespace std::chrono;
using namespace ncast;

// Configuration
const size_t ITERATIONS = 50000000;  // 50 million iterations for ~5 seconds per run
const size_t WARMUP_ITERATIONS = 5000000;  // 5 million iterations for warm-up
const int DEFAULT_RUNS = 5;  // Default number of benchmark runs

struct BenchmarkStats {
    std::string name;
    std::vector<double> times;
    double average;
    double median;
    double std_dev;
    double min_time;
    double max_time;
    
    void calculate_stats() {
        if (times.empty()) return;
        
        // Sort for median calculation
        std::vector<double> sorted_times = times;
        std::sort(sorted_times.begin(), sorted_times.end());
        
        // Calculate average
        average = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        
        // Calculate median
        size_t n = sorted_times.size();
        if (n % 2 == 0) {
            median = (sorted_times[n/2 - 1] + sorted_times[n/2]) / 2.0;
        } else {
            median = sorted_times[n/2];
        }
        
        // Calculate standard deviation
        double sum_sq_diff = 0.0;
        for (double time : times) {
            double diff = time - average;
            sum_sq_diff += diff * diff;
        }
        std_dev = std::sqrt(sum_sq_diff / times.size());
        
        // Min and max
        min_time = *std::min_element(times.begin(), times.end());
        max_time = *std::max_element(times.begin(), times.end());
    }
};

class BenchmarkTimer {
private:
    high_resolution_clock::time_point start_time;
    
public:
    void start() {
        start_time = high_resolution_clock::now();
    }
    
    double stop() {
        auto end_time = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end_time - start_time);
        return static_cast<double>(duration.count()) / 1000.0; // Return milliseconds
    }
};

// Heavy computation function using static_cast
double heavy_computation_static_cast(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        // Simulate heavy numeric computation with casting
        int casted_value = static_cast<int>(value);
        unsigned int unsigned_val = static_cast<unsigned int>(std::abs(casted_value));
        short short_val = static_cast<short>(unsigned_val % 32767);
        
        // Some mathematical operations
        double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
        result += temp * static_cast<double>(i % 1000);
    }
    
    return result;
}

// Heavy computation function using numeric_cast without validation
double heavy_computation_ncast_no_validation(const std::vector<long>& data) {
    return heavy_computation_ncast_no_validation_real(data, ITERATIONS);
}

// Heavy computation function using numeric_cast with validation
double heavy_computation_ncast_with_validation(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        try {
            int casted_value = numeric_cast<int>(value);
            unsigned int unsigned_val = numeric_cast<unsigned int>(std::abs(casted_value));
            short short_val = numeric_cast<short>(unsigned_val % 32767);
            
            double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
            result += temp * static_cast<double>(i % 1000);
        } catch (const cast_exception&) {
            result += 0.1 * static_cast<double>(i % 1000);
        }
    }
    
    return result;
}

// Heavy computation function using NUMERIC_CAST macro without validation
double heavy_computation_macro_no_validation(const std::vector<long>& data) {
    return heavy_computation_macro_no_validation_real(data, ITERATIONS);
}

// Heavy computation function using NUMERIC_CAST macro with validation
double heavy_computation_macro_with_validation(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        try {
            int casted_value = NUMERIC_CAST(int, value);
            unsigned int unsigned_val = NUMERIC_CAST(unsigned int, std::abs(casted_value));
            short short_val = NUMERIC_CAST(short, unsigned_val % 32767);
            
            double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
            result += temp * static_cast<double>(i % 1000);
        } catch (const cast_exception&) {
            result += 0.1 * static_cast<double>(i % 1000);
        }
    }
    
    return result;
}

// Generate test data that will work with all casting methods
std::vector<long> generate_test_data() {
    std::vector<long> data;
    data.reserve(10000);
    
    std::mt19937 gen(42); // Fixed seed for reproducible results
    std::uniform_int_distribution<long> dis(-100000, 100000);
    
    for (int i = 0; i < 10000; ++i) {
        data.push_back(dis(gen));
    }
    
    return data;
}

// Warm-up function (runs with fewer iterations)
template<typename Func>
void warmup_function(Func func, const std::vector<long>& data) {
    // Run a smaller version for warm-up
    std::vector<long> warmup_data(data.begin(), data.begin() + std::min(size_t(1000), data.size()));
    volatile double result = func(warmup_data);
    (void)result; // Suppress unused variable warning
}

// Benchmark a single function multiple times
template<typename Func>
BenchmarkStats benchmark_function(const std::string& name, Func func, 
                                  const std::vector<long>& data, int num_runs) {
    BenchmarkStats stats;
    stats.name = name;
    stats.times.reserve(num_runs);
    
    BenchmarkTimer timer;
    
    std::cout << "Benchmarking " << name << " (" << num_runs << " runs):" << std::endl;
    
    // Warm-up
    std::cout << "  Warming up..." << std::flush;
    warmup_function(func, data);
    std::cout << " done" << std::endl;
    
    // Run benchmarks
    for (int run = 0; run < num_runs; ++run) {
        std::cout << "  Run " << (run + 1) << "/" << num_runs << "..." << std::flush;
        
        timer.start();
        volatile double result = func(data);
        double time = timer.stop();
        (void)result; // Suppress unused variable warning
        
        stats.times.push_back(time);
        std::cout << " " << std::fixed << std::setprecision(1) << time << "ms" << std::endl;
    }
    
    stats.calculate_stats();
    std::cout << std::endl;
    return stats;
}

// Display comprehensive statistics table
void display_statistics(const std::vector<BenchmarkStats>& all_stats) {
    std::cout << "=== Comprehensive Statistics (all times in ms) ===" << std::endl;
    std::cout << std::setw(25) << "Method" 
              << std::setw(10) << "Average" 
              << std::setw(10) << "Median"
              << std::setw(10) << "StdDev"
              << std::setw(10) << "Min"
              << std::setw(10) << "Max" << std::endl;
    std::cout << std::string(75, '-') << std::endl;
    
    for (const auto& stats : all_stats) {
        std::cout << std::setw(25) << stats.name
                  << std::setw(10) << std::fixed << std::setprecision(1) << stats.average
                  << std::setw(10) << std::fixed << std::setprecision(1) << stats.median
                  << std::setw(10) << std::fixed << std::setprecision(1) << stats.std_dev
                  << std::setw(10) << std::fixed << std::setprecision(1) << stats.min_time
                  << std::setw(10) << std::fixed << std::setprecision(1) << stats.max_time
                  << std::endl;
    }
    std::cout << std::endl;
}

// Display overhead analysis
void display_overhead_analysis(const std::vector<BenchmarkStats>& all_stats) {
    if (all_stats.empty()) return;
    
    const auto& baseline = all_stats[0]; // static_cast is baseline
    
    std::cout << "=== Overhead Analysis (relative to static_cast baseline) ===" << std::endl;
    
    for (size_t i = 0; i < all_stats.size(); ++i) {
        const auto& stats = all_stats[i];
        
        if (i == 0) {
            std::cout << stats.name << ": baseline (1.0x)" << std::endl;
        } else {
            double relative_perf = stats.average / baseline.average;
            double overhead_pct = ((stats.average - baseline.average) / baseline.average) * 100.0;
            
            std::cout << stats.name << ": " 
                      << std::fixed << std::setprecision(1) << relative_perf << "x";
            
            if (overhead_pct > 0.1) {
                std::cout << " (+" << std::setprecision(1) << overhead_pct << "% overhead)";
            } else {
                std::cout << " (negligible overhead)";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

// Display compact summary for README
void display_readme_format(const std::vector<BenchmarkStats>& all_stats, int num_runs) {
    std::cout << "=== README Format Summary ===" << std::endl;
    std::cout << "```" << std::endl;
    std::cout << "=== Performance Summary (Average of " << num_runs << " runs) ===" << std::endl;
    
    for (size_t i = 0; i < all_stats.size(); ++i) {
        const auto& stats = all_stats[i];
        std::cout << (i + 1) << ". " << std::setw(28) << std::left << (stats.name + ":")
                  << std::right << std::setw(8) << std::fixed << std::setprecision(1) 
                  << stats.average << " ms (±" 
                  << std::setprecision(1) << stats.std_dev << ")" << std::endl;
    }
    
    // Calculate overheads for validation cases
    if (all_stats.size() >= 5) {
        const auto& baseline = all_stats[0];
        const auto& func_val = all_stats[2];
        const auto& macro_val = all_stats[4];
        
        double func_overhead = ((func_val.average - baseline.average) / baseline.average) * 100.0;
        double macro_overhead = ((macro_val.average - baseline.average) / baseline.average) * 100.0;
        
        std::cout << std::endl;
        std::cout << "Function validation overhead: " << std::setprecision(1) << func_overhead << "%" << std::endl;
        std::cout << "Macro validation overhead:    " << std::setprecision(1) << macro_overhead << "%" << std::endl;
    }
    
    std::cout << "```" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    int num_runs = DEFAULT_RUNS;
    if (argc > 1) {
        num_runs = std::atoi(argv[1]);
        if (num_runs <= 0) {
            std::cerr << "Error: Number of runs must be positive" << std::endl;
            return 1;
        }
    }
    
    std::cout << "ncast Performance Benchmark with Statistical Analysis" << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout << "Iterations per run: " << ITERATIONS << std::endl;
    std::cout << "Number of runs: " << num_runs << std::endl;
    std::cout << "Statistical analysis: average, median, std dev, min, max" << std::endl;
    std::cout << std::endl;
    
    // Generate test data
    std::cout << "Generating test data..." << std::endl;
    auto test_data = generate_test_data();
    std::cout << std::endl;
    
    // Run all benchmarks
    std::vector<BenchmarkStats> all_stats;
    
    all_stats.push_back(benchmark_function("static_cast", 
        heavy_computation_static_cast, test_data, num_runs));
    
    all_stats.push_back(benchmark_function("numeric_cast (no validation)", 
        heavy_computation_ncast_no_validation, test_data, num_runs));
    
    all_stats.push_back(benchmark_function("numeric_cast (validation)", 
        heavy_computation_ncast_with_validation, test_data, num_runs));
    
    all_stats.push_back(benchmark_function("NUMERIC_CAST (no validation)", 
        heavy_computation_macro_no_validation, test_data, num_runs));
    
    all_stats.push_back(benchmark_function("NUMERIC_CAST (validation)", 
        heavy_computation_macro_with_validation, test_data, num_runs));
    
    // Display all results
    display_statistics(all_stats);
    display_overhead_analysis(all_stats);
    display_readme_format(all_stats, num_runs);
    
    std::cout << "Benchmark completed successfully!" << std::endl;
    
    return 0;
}
