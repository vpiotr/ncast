/**
 * @file benchmark_ncast_no_validation.cpp
 * @brief Benchmark module compiled with validation disabled
 * 
 * This module is compiled with NCAST_DISABLE_VALIDATION defined via CMake,
 * allowing us to test the true performance of number_cast without validation.
 */

#include "../include/ncast/ncast.h"
#include <vector>
#include <cmath>

using namespace ncast;

/**
 * @brief Heavy computation function using number_cast with validation disabled
 * 
 * This function uses the real number_cast function, but with validation
 * disabled at compile time via NCAST_DISABLE_VALIDATION.
 */
double heavy_computation_ncast_no_validation_real(const std::vector<long>& data, size_t iterations) {
    double result = 0.0;
    
    for (size_t i = 0; i < iterations; ++i) {
        long value = data[i % data.size()];
        
        // Use real number_cast with validation disabled
        int casted_value = number_cast<int>(value);
        unsigned int unsigned_val = number_cast<unsigned int>(std::abs(casted_value));
        short short_val = number_cast<short>(unsigned_val % 32767);
        
        // Some mathematical operations (must match the other benchmark functions)
        double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
        result += temp * static_cast<double>(i % 1000);
    }
    
    return result;
}

/**
 * @brief Heavy computation function using NUMBER_CAST macro with validation disabled
 * 
 * This function uses the real NUMBER_CAST macro, but with validation
 * disabled at compile time via NCAST_DISABLE_VALIDATION.
 */
double heavy_computation_macro_no_validation_real(const std::vector<long>& data, size_t iterations) {
    double result = 0.0;
    
    for (size_t i = 0; i < iterations; ++i) {
        long value = data[i % data.size()];
        
        // Use real NUMBER_CAST macro with validation disabled
        int casted_value = NUMBER_CAST(int, value);
        unsigned int unsigned_val = NUMBER_CAST(unsigned int, std::abs(casted_value));
        short short_val = NUMBER_CAST(short, unsigned_val % 32767);
        
        // Some mathematical operations (must match the other benchmark functions)
        double temp = std::sin(short_val * 0.001) + std::cos(unsigned_val * 0.0001);
        result += temp * static_cast<double>(i % 1000);
    }
    
    return result;
}
