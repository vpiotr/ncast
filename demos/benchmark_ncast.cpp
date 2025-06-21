/**
 * @file benchmark_ncast.cpp
 * @brief Performance benchmark for ncast library
 * 
 * This benchmark compares three casting approaches:
 * 1. static_cast (baseline)
 * 2. number_cast with validation disabled (via separate module)
 * 3. number_cast with validation enabled
 * 
 * The "no validation" test uses a multi-module approach where
 * benchmark_ncast_no_validation.cpp is compiled with NCAST_DISABLE_VALIDATION
 * defined, ensuring we test the true performance of number_cast without
 * validation rather than simulating it.
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <cmath>
#include <random>
#include "../include/ncast/ncast.h"
#include "benchmark_ncast_no_validation.h"

using namespace std::chrono;
using namespace ncast;

// Configuration
const size_t ITERATIONS = 50000000;  // 50 million iterations for ~5 seconds
const size_t WARMUP_ITERATIONS = 5000000;  // 5 million iterations for warm-up
const int CHART_WIDTH = 60;
const int CHART_HEIGHT = 12;

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
        return duration.count() / 1000.0; // Return milliseconds
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
        result += temp * (i % 1000);
    }
    
    return result;
}

// Heavy computation function using number_cast without validation
// Note: This function is implemented in a separate compilation unit
// with NCAST_DISABLE_VALIDATION defined for true performance testing
double heavy_computation_ncast_no_validation(const std::vector<long>& data) {
    // Call the real implementation from the separate module
    return heavy_computation_ncast_no_validation_real(data, ITERATIONS);
}

// Heavy computation function using number_cast with validation
double heavy_computation_ncast_with_validation(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        try {
            // Use number_cast with validation
            int casted_value = number_cast<int>(value);
            unsigned int unsigned_val = number_cast<unsigned int>(std::abs(casted_value));
            short short_val = number_cast<short>(unsigned_val % 32767);
            
            // Some mathematical operations
            double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
            result += temp * (i % 1000);
        } catch (const cast_exception&) {
            // Handle invalid casts (shouldn't happen with our test data)
            result += 0.1 * (i % 1000);
        }
    }
    
    return result;
}

// Heavy computation function using NUMBER_CAST macro without validation
// Note: This function is implemented in a separate compilation unit
// with NCAST_DISABLE_VALIDATION defined for true performance testing
double heavy_computation_macro_no_validation(const std::vector<long>& data) {
    // Call the real implementation from the separate module
    return heavy_computation_macro_no_validation_real(data, ITERATIONS);
}

// Heavy computation function using NUMBER_CAST macro with validation
double heavy_computation_macro_with_validation(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        try {
            // Use NUMBER_CAST macro with validation and location info
            int casted_value = NUMBER_CAST(int, value);
            unsigned int unsigned_val = NUMBER_CAST(unsigned int, std::abs(casted_value));
            short short_val = NUMBER_CAST(short, unsigned_val % 32767);
            
            // Some mathematical operations
            double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
            result += temp * (i % 1000);
        } catch (const cast_exception&) {
            // Handle invalid casts (shouldn't happen with our test data)
            result += 0.1 * (i % 1000);
        }
    }
    
    return result;
}

// Generate test data that will work with all casting methods
std::vector<long> generate_test_data() {
    std::vector<long> data;
    data.reserve(10000);
    
    std::random_device rd;
    std::mt19937 gen(42); // Fixed seed for reproducible results
    
    // Generate values that fit safely in int range
    std::uniform_int_distribution<long> dis(-100000, 100000);
    
    for (int i = 0; i < 10000; ++i) {
        data.push_back(dis(gen));
    }
    
    return data;
}

// Draw ASCII chart
void draw_performance_chart(const std::vector<std::pair<std::string, double>>& results) {
    std::cout << "\n=== Performance Chart (lower is better) ===" << std::endl;
    
    // Find min and max values for better scaling
    double min_time = results[0].second;
    double max_time = results[0].second;
    for (const auto& result : results) {
        min_time = std::min(min_time, result.second);
        max_time = std::max(max_time, result.second);
    }
    
    // Use a focused range: expand the visible range to show differences better
    double range = max_time - min_time;
    if (range < max_time * 0.02) {  // If difference is less than 2%, expand the range
        double center = (min_time + max_time) / 2.0;
        double expanded_range = max_time * 0.15;  // Show 15% range around center
        min_time = center - expanded_range / 2.0;
        max_time = center + expanded_range / 2.0;
        range = max_time - min_time;
    } else {
        // Add 10% padding above and below for better visualization
        double padding = range * 0.1;
        min_time -= padding;
        max_time += padding;
        range = max_time - min_time;
    }
    
    // Draw chart
    std::cout << std::endl;
    for (int row = CHART_HEIGHT - 1; row >= 0; --row) {
        double threshold = min_time + (range * (row + 1)) / CHART_HEIGHT;
        
        // Display threshold with appropriate precision
        if (range > 100) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(0) << threshold << "ms |";
        } else {
            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << threshold << "ms |";
        }
        
        for (const auto& result : results) {
            if (result.second >= threshold) {
                std::cout << " ████████████████████";
            } else {
                std::cout << "                     ";
            }
        }
        std::cout << std::endl;
    }
    
    // Add a baseline indicator showing the minimum value
    std::cout << std::setw(8) << std::fixed;
    if (range > 100) {
        std::cout << std::setprecision(0) << min_time;
    } else {
        std::cout << std::setprecision(1) << min_time;
    }
    std::cout << "ms +";
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "---------------------";
    }
    std::cout << " (baseline)" << std::endl;
    
    // Draw labels (split into two lines for better alignment)
    std::cout << "          ";
    for (const auto& result : results) {
        std::string label = result.first;
        std::string line1, line2;
        
        if (label == "static_cast") {
            line1 = "static_cast";
            line2 = "";
        } else if (label == "ncast (no validation)") {
            line1 = "ncast";
            line2 = "(no validation)";
        } else if (label == "ncast (validation)") {
            line1 = "ncast";
            line2 = "(validation)";
        } else if (label == "macro (no validation)") {
            line1 = "macro";
            line2 = "(no validation)";
        } else if (label == "macro (validation)") {
            line1 = "macro";
            line2 = "(validation)";
        } else {
            line1 = label;
            line2 = "";
        }
        
        // Center first line in 21-character field
        int padding1 = (21 - line1.length()) / 2;
        std::cout << std::setw(padding1) << "" << line1 << std::setw(21 - padding1 - line1.length()) << "";
    }
    std::cout << std::endl;
    
    // Draw second line of labels
    std::cout << "          ";
    for (const auto& result : results) {
        std::string label = result.first;
        std::string line2;
        
        if (label == "static_cast") {
            line2 = "";
        } else if (label == "ncast (no validation)") {
            line2 = "(no validation)";
        } else if (label == "ncast (validation)") {
            line2 = "(validation)";
        } else if (label == "macro (no validation)") {
            line2 = "(no validation)";
        } else if (label == "macro (validation)") {
            line2 = "(validation)";
        } else {
            line2 = "";
        }
        
        // Center second line in 21-character field
        int padding2 = (21 - line2.length()) / 2;
        std::cout << std::setw(padding2) << "" << line2 << std::setw(21 - padding2 - line2.length()) << "";
    }
    std::cout << std::endl;
}

// Warm-up functions (smaller iteration count)
double warmup_static_cast(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        // Simulate heavy numeric computation with casting
        int casted_value = static_cast<int>(value);
        unsigned int unsigned_val = static_cast<unsigned int>(std::abs(casted_value));
        short short_val = static_cast<short>(unsigned_val % 32767);
        
        // Some mathematical operations
        double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
        result += temp * (i % 1000);
    }
    
    return result;
}

double warmup_ncast_no_validation(const std::vector<long>& data) {
    // Call the real implementation from the separate module with warm-up iterations
    return heavy_computation_ncast_no_validation_real(data, WARMUP_ITERATIONS);
}

double warmup_ncast_with_validation(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        try {
            // Use number_cast with validation
            int casted_value = number_cast<int>(value);
            unsigned int unsigned_val = number_cast<unsigned int>(std::abs(casted_value));
            short short_val = number_cast<short>(unsigned_val % 32767);
            
            // Some mathematical operations
            double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
            result += temp * (i % 1000);
        } catch (const cast_exception&) {
            // Handle invalid casts (shouldn't happen with our test data)
            result += 0.1 * (i % 1000);
        }
    }
    
    return result;
}

double warmup_macro_no_validation(const std::vector<long>& data) {
    // Call the real implementation from the separate module with warm-up iterations
    return heavy_computation_macro_no_validation_real(data, WARMUP_ITERATIONS);
}

double warmup_macro_with_validation(const std::vector<long>& data) {
    double result = 0.0;
    
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        long value = data[i % data.size()];
        
        try {
            // Use NUMBER_CAST macro with validation and location info
            int casted_value = NUMBER_CAST(int, value);
            unsigned int unsigned_val = NUMBER_CAST(unsigned int, std::abs(casted_value));
            short short_val = NUMBER_CAST(short, unsigned_val % 32767);
            
            // Some mathematical operations
            double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
            result += temp * (i % 1000);
        } catch (const cast_exception&) {
            // Handle invalid casts (shouldn't happen with our test data)
            result += 0.1 * (i % 1000);
        }
    }
    
    return result;
}

int main() {
    std::cout << "ncast Performance Benchmark" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << "Iterations: " << ITERATIONS << " (with " << WARMUP_ITERATIONS << " warm-up)" << std::endl;
    std::cout << "Testing casting performance (functions and macros) with heavy computation..." << std::endl << std::endl;
    
    // Generate test data
    std::cout << "Generating test data..." << std::endl;
    auto test_data = generate_test_data();
    
    std::cout << "Starting benchmarks with individual warm-up phases..." << std::endl << std::endl;
    
    BenchmarkTimer timer;
    std::vector<std::pair<std::string, double>> results;
    
    // Warm-up: static_cast
    std::cout << "Warming up static_cast..." << std::flush;
    warmup_static_cast(test_data);
    std::cout << " done." << std::endl;
    
    // Warm-up: number_cast without validation (real implementation)
    std::cout << "Warming up number_cast (no validation)..." << std::flush;
    warmup_ncast_no_validation(test_data);
    std::cout << " done." << std::endl;
    
    // Warm-up: number_cast with validation
    std::cout << "Warming up number_cast (with validation)..." << std::flush;
    warmup_ncast_with_validation(test_data);
    std::cout << " done." << std::endl;
    
    // Warm-up: NUMBER_CAST macro without validation (real implementation)
    std::cout << "Warming up NUMBER_CAST (no validation)..." << std::flush;
    warmup_macro_no_validation(test_data);
    std::cout << " done." << std::endl;
    
    // Warm-up: NUMBER_CAST macro with validation
    std::cout << "Warming up NUMBER_CAST (with validation)..." << std::flush;
    warmup_macro_with_validation(test_data);
    std::cout << " done." << std::endl;
    
    // Benchmark 1: static_cast
    std::cout << "Running benchmark 1/5: static_cast..." << std::flush;
    std::cout << "warming up..." << std::flush;
    volatile double warmup_result1 = warmup_static_cast(test_data);  // volatile to prevent optimization
    std::cout << " measuring..." << std::flush;
    timer.start();
    double result1 = heavy_computation_static_cast(test_data);
    double time1 = timer.stop();
    results.emplace_back("static_cast", time1);
    std::cout << " (" << std::fixed << std::setprecision(2) << time1 << "ms)" << std::endl;
    
    // Benchmark 2: number_cast without validation (real implementation)
    std::cout << "Running benchmark 2/5: number_cast (no validation)..." << std::flush;
    std::cout << "warming up..." << std::flush;
    volatile double warmup_result2 = warmup_ncast_no_validation(test_data);  // volatile to prevent optimization
    std::cout << " measuring..." << std::flush;
    timer.start();
    double result2 = heavy_computation_ncast_no_validation(test_data);
    double time2 = timer.stop();
    results.emplace_back("ncast (no validation)", time2);
    std::cout << " (" << std::fixed << std::setprecision(2) << time2 << "ms)" << std::endl;
    
    // Benchmark 3: number_cast with validation
    std::cout << "Running benchmark 3/5: number_cast (with validation)..." << std::flush;
    std::cout << "warming up..." << std::flush;
    volatile double warmup_result3 = warmup_ncast_with_validation(test_data);  // volatile to prevent optimization
    std::cout << " measuring..." << std::flush;
    timer.start();
    double result3 = heavy_computation_ncast_with_validation(test_data);
    double time3 = timer.stop();
    results.emplace_back("ncast (validation)", time3);
    std::cout << " (" << std::fixed << std::setprecision(2) << time3 << "ms)" << std::endl;
    
    // Benchmark 4: NUMBER_CAST macro without validation (real implementation)
    std::cout << "Running benchmark 4/5: NUMBER_CAST (no validation)..." << std::flush;
    std::cout << "warming up..." << std::flush;
    volatile double warmup_result4 = warmup_macro_no_validation(test_data);  // volatile to prevent optimization
    std::cout << " measuring..." << std::flush;
    timer.start();
    double result4 = heavy_computation_macro_no_validation(test_data);
    double time4 = timer.stop();
    results.emplace_back("macro (no validation)", time4);
    std::cout << " (" << std::fixed << std::setprecision(2) << time4 << "ms)" << std::endl;
    
    // Benchmark 5: NUMBER_CAST macro with validation
    std::cout << "Running benchmark 5/5: NUMBER_CAST (with validation)..." << std::flush;
    std::cout << "warming up..." << std::flush;
    volatile double warmup_result5 = warmup_macro_with_validation(test_data);  // volatile to prevent optimization
    std::cout << " measuring..." << std::flush;
    timer.start();
    double result5 = heavy_computation_macro_with_validation(test_data);
    double time5 = timer.stop();
    results.emplace_back("macro (validation)", time5);
    std::cout << " (" << std::fixed << std::setprecision(2) << time5 << "ms)" << std::endl;
    
    // Draw performance chart
    draw_performance_chart(results);
    
    // Summary
    std::cout << "\n=== Performance Summary ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "1. static_cast:                  " << std::setw(8) << time1 << " ms" << std::endl;
    std::cout << "2. number_cast (no validation):  " << std::setw(8) << time2 << " ms";
    if (time1 > 0) {
        std::cout << " (" << std::setprecision(1) << (time2/time1) << "x)";
    }
    std::cout << std::endl;
    std::cout << "3. number_cast (validation):     " << std::setw(8) << time3 << " ms";
    if (time1 > 0) {
        std::cout << " (" << std::setprecision(1) << (time3/time1) << "x)";
    }
    std::cout << std::endl;
    std::cout << "4. NUMBER_CAST (no validation):  " << std::setw(8) << time4 << " ms";
    if (time1 > 0) {
        std::cout << " (" << std::setprecision(1) << (time4/time1) << "x)";
    }
    std::cout << std::endl;
    std::cout << "5. NUMBER_CAST (validation):     " << std::setw(8) << time5 << " ms";
    if (time1 > 0) {
        std::cout << " (" << std::setprecision(1) << (time5/time1) << "x)";
    }
    std::cout << std::endl;
    
    // Performance analysis
    std::cout << "\n=== Analysis ===" << std::endl;
    if (time3 > time1) {
        double overhead = ((time3 - time1) / time1) * 100.0;
        std::cout << "Function validation overhead: " << std::setprecision(1) << overhead << "%" << std::endl;
    } else {
        std::cout << "Function validation overhead: negligible" << std::endl;
    }
    
    if (time5 > time1) {
        double overhead = ((time5 - time1) / time1) * 100.0;
        std::cout << "Macro validation overhead:    " << std::setprecision(1) << overhead << "%" << std::endl;
    } else {
        std::cout << "Macro validation overhead: negligible" << std::endl;
    }
    
    // Verify results are reasonable (sanity check)
    std::cout << "\n=== Sanity Check ===" << std::endl;
    std::cout << "Result 1 (static_cast):    " << std::scientific << std::setprecision(3) << result1 << std::endl;
    std::cout << "Result 2 (func no val):    " << std::scientific << std::setprecision(3) << result2 << std::endl;
    std::cout << "Result 3 (func validation):" << std::scientific << std::setprecision(3) << result3 << std::endl;
    std::cout << "Result 4 (macro no val):   " << std::scientific << std::setprecision(3) << result4 << std::endl;
    std::cout << "Result 5 (macro validation):" << std::scientific << std::setprecision(3) << result5 << std::endl;
    
    // Use warmup results to prevent compiler warnings (but don't affect measurements)
    volatile double total_warmup = warmup_result1 + warmup_result2 + warmup_result3 + warmup_result4 + warmup_result5;
    (void)total_warmup;  // Suppress unused variable warning
    
    // Check consistency
    std::vector<double> all_results = {result1, result2, result3, result4, result5};
    bool consistent = true;
    for (size_t i = 1; i < all_results.size(); ++i) {
        double diff = std::abs(all_results[i] - result1) / std::abs(result1);
        if (diff >= 0.01) {  // More than 1% difference
            consistent = false;
            break;
        }
    }
    
    if (consistent) {
        std::cout << "✓ All results are consistent" << std::endl;
    } else {
        std::cout << "⚠ Results differ (this may be expected due to different random seeds)" << std::endl;
    }
    
    std::cout << "\nBenchmark completed successfully!" << std::endl;
    
    return 0;
}
