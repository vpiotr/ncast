/**
 * @file benchmark_ncast_no_validation.h
 * @brief Header for the no-validation benchmark module
 */

#ifndef BENCHMARK_NCAST_NO_VALIDATION_H
#define BENCHMARK_NCAST_NO_VALIDATION_H

#include <vector>

/**
 * @brief Heavy computation function using numeric_cast with validation disabled
 * 
 * This function is implemented in a separate compilation unit with
 * NCAST_DISABLE_VALIDATION defined, allowing true testing of numeric_cast
 * performance without validation overhead.
 */
double heavy_computation_ncast_no_validation_real(const std::vector<long>& data, size_t iterations);

/**
 * @brief Heavy computation function using NUMERIC_CAST macro with validation disabled
 * 
 * This function is implemented in a separate compilation unit with
 * NCAST_DISABLE_VALIDATION defined, allowing true testing of NUMERIC_CAST
 * macro performance without validation overhead.
 */
double heavy_computation_macro_no_validation_real(const std::vector<long>& data, size_t iterations);

#endif // BENCHMARK_NCAST_NO_VALIDATION_H
