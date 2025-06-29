# ncast - Safe Numeric Casting Library

[![Build Status](https://github.com/vpiotr/ncast/actions/workflows/ci.yml/badge.svg)](https://github.com/vpiotr/ncast/actions/workflows/ci.yml)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11%2B-blue)](https://en.cppreference.com/w/cpp/11)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Header Only](https://img.shields.io/badge/header--only-yes-orange)](include/ncast/ncast.h)
[![Platform](https://img.shields.io/badge/platform-cross--platform-lightgrey)](CMakeLists.txt)

A header-only C++ micro-library for safe numeric casting with runtime validation, optional compile-time validation (C++14+), and comprehensive error reporting.

## Quick Overview

**ncast** solves the problem of unsafe numeric type conversions in C++. In standard C++, using `static_cast` for numeric conversions can lead to silent data corruption, unexpected behavior, and hard-to-find bugs due to:

- Negative values being converted to unsigned types
- Values that exceed the target type's range
- Special floating-point values (NaN, infinity) being improperly converted
- Unexpected sign conversions

This library provides `numeric_cast<T>()` and `NUMERIC_CAST(T, val)` which perform comprehensive validation before converting values, throwing informative exceptions when conversions would be unsafe. In C++14+, validation can occur at compile time for constant expressions, while maintaining full C++11 compatibility with runtime validation. With validation disabled, it compiles down to a simple `static_cast` with zero overhead.

```cpp
// Instead of error-prone static_cast:
unsigned int result = static_cast<unsigned int>(-1);  // Silently becomes a large value!

// Use safe numeric_cast:
unsigned int result = numeric_cast<unsigned int>(value);  // Runtime validation (all standards)

// C++14+ compile-time validation for constants:
constexpr unsigned int compile_time_result = numeric_cast<unsigned int>(42);  // Compile-time validation
```

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Building](#building)
- [API Reference](#api-reference)
- [Examples](#examples)
- [Testing](#testing)
- [Benchmarks](#benchmarks)
- [Documentation](#documentation)
- [Performance](#performance)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)

## Features

- **Safe casting**: Validates value ranges before casting to prevent dangerous conversions
- **C++11 compatible**: Full functionality available from C++11 onwards
- **Optional compile-time validation**: C++14+ enables compile-time validation for constant expressions
- **Header-only**: Just include `ncast.h` and start using - no linking required
- **Comprehensive support**: Works with all numeric types plus char types
- **Enhanced precision**: Uses centralized widening types for maximum accuracy in range validation
- **Enhanced long double support**: Uses high-precision intermediate calculations for accurate range validation
- **Special floating-point handling**: Properly manages NaN, infinity, and denormal values
- **Two APIs**: Function templates and macros with precise location information
- **Optional validation**: Can be disabled for performance-critical code paths
- **Exception safety**: Clear error messages with file/line/function information
- **Zero overhead**: When validation is disabled, performs identical to `static_cast`
- **Boundary testing**: Uses `std::numeric_limits` for accurate range validation
- **Modular testing**: Well-organized test suite with focused test modules for maintainability
- **High performance**: Minimal overhead with extensive benchmarking

## Installation

### Option 1: Direct Include (Recommended)
Simply copy `include/ncast/ncast.h` to your project and include it:

```cpp
#include "ncast.h"  // or #include <ncast/ncast.h>
using namespace ncast;
```

### Option 2: CMake Integration
Add ncast as a subdirectory or use CMake's package system:

```cmake
# Add as subdirectory
add_subdirectory(ncast)
target_link_libraries(your_target ncast)

# Or find installed package
find_package(ncast REQUIRED)
target_link_libraries(your_target ncast::ncast)
```

### Option 3: System Installation
```bash
git clone https://github.com/vpiotr/ncast.git
cd ncast
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make install
```

## Quick Start

```cpp
#include <ncast/ncast.h>
using namespace ncast;

// Safe casting - throws cast_exception if value doesn't fit (all C++ standards)
int value = 42;
unsigned int result = numeric_cast<unsigned int>(value);  // Runtime validation

// This would throw because -1 cannot be safely cast to unsigned
int negative = -1;
unsigned int bad_result = numeric_cast<unsigned int>(negative);  // Throws!

// C++14+ compile-time validation for constant expressions
constexpr int compile_time_value = 42;
constexpr auto compile_time_result = numeric_cast<unsigned int>(compile_time_value);  // Compile-time validation

// Char-specific casting (always safe between char types)
signed char sc = 'A';
unsigned char uc = char_cast<unsigned char>(sc);  // Always safe

// C++14+ compile-time char casting
constexpr auto compile_time_char = char_cast<unsigned char>('B');  // Compile-time

// Macro versions provide better error location information
auto result2 = NUMERIC_CAST(unsigned int, value);
auto result3 = CHAR_CAST(unsigned char, sc);
```

## Building

### Requirements

- C++11 compatible compiler
- CMake 3.10 or later
- Optional: Doxygen for documentation

### Build Steps

```bash
# Clone and build
git clone https://github.com/vpiotr/ncast.git
cd ncast

# Build everything from scratch
./rebuild.sh

# Run tests
./run_tests.sh

# Run demos  
./run_demos.sh

# Run benchmarks
./run_benchmarks.sh

# Build documentation (requires Doxygen)
./build_docs.sh
```

### CMake Options

```bash
# Build with tests (default: ON)
cmake .. -DBUILD_TESTS=ON

# Build with demos and benchmarks (default: ON)  
cmake .. -DBUILD_DEMOS=ON

# Build documentation (default: ON if Doxygen found)
cmake .. -DBUILD_DOCS=ON

# Disable runtime validation globally (default: OFF)
cmake .. -DNCAST_DISABLE_RUNTIME_VALIDATION=ON

# Release build for benchmarks
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## API Reference

### numeric_cast

Safe casting between any numeric types and char with optional compile-time validation:

```cpp
template<typename ToType, typename FromType>
ToType numeric_cast(FromType value);                    // C++11: runtime validation
                                                        // C++14+: compile-time validation for constants

// Macro version with location info
#define NUMERIC_CAST(ToType, value)
```

**Validation modes:**
- **C++11**: Runtime validation only - fully functional base library
- **C++14+**: Optional compile-time validation for constant expressions, with automatic fallback to runtime validation for non-constants

**Supported types:**
- All integral types: `bool`, `char`, `signed char`, `unsigned char`, `short`, `unsigned short`, `int`, `unsigned int`, `long`, `unsigned long`, `long long`, `unsigned long long`
- All floating-point types: `float`, `double`, `long double`
- **NOT supported**: Pointer types, user-defined types, arrays, references (compile-time error via `static_assert`)

**Validation rules:**
- Negative values cannot be cast to unsigned types
- Values must fit within target type's range (uses `std::numeric_limits`)
- **High-precision range checking**: Uses centralized widening types (`long double` for floating-point, `long long` for integer comparisons) for maximum accuracy, ensuring proper validation even when converting to/from `long double` types
- Special floating-point values are handled properly:
  - NaN can only be converted between floating-point types
  - Infinity can only be converted between floating-point types
  - Denormal values are properly validated for range
- Throws `cast_exception` on validation failure with detailed context
- Compile-time type safety enforced via `static_assert`

**Compile-time usage (C++14+):**
```cpp
constexpr int value = 42;
constexpr auto result = numeric_cast<unsigned int>(value);  // Validated at compile time
```

**Exception details:**
```cpp
// Exception provides rich context information
try {
    auto result = NUMERIC_CAST(unsigned int, -42);
} catch (const cast_exception& e) {
    // e.what() includes value, types, file, line, and function
    // e.getFile(), e.getLine(), e.getFunction() for programmatic access
}
```

### char_cast

Safe casting between char types only with optional compile-time evaluation:

```cpp
template<typename ToType, typename FromType>
ToType char_cast(FromType value);                       // C++11: runtime execution
                                                        // C++14+: compile-time evaluation

// Macro version with location info  
#define CHAR_CAST(ToType, value)
```

**Features:**
- Works only with `char`, `signed char`, `unsigned char`
- Always safe - no runtime validation needed (performs a simple value conversion)
- **C++14+**: Can be evaluated at compile time for constant expressions
- Compiler error if used with non-char types
- Zero overhead - pure compile-time safety

**Compile-time usage (C++14+):**
```cpp
constexpr auto result = char_cast<unsigned char>('A');  // Evaluated at compile time
```

**Why char_cast?**
Character types have special conversion semantics in C++. This function provides a safe, explicit way to convert between character types without the overhead of runtime validation, since all char-to-char conversions are well-defined.

### cast_exception

Rich exception class with comprehensive error information:

```cpp
class cast_exception : public std::runtime_error {
public:
    const char* getFile() const;     // Source file where cast failed
    int getLine() const;             // Line number of failed cast
    const char* getFunction() const; // Function name where cast failed
};
```

### C++ Standard Compatibility

**ncast** is designed to provide maximum functionality across all C++ standards while enabling enhanced features for newer standards:

**C++11 (Base functionality):**
- Full runtime validation for all numeric conversions
- Complete exception handling with location information
- All core features: `numeric_cast`, `char_cast`, macros
- Zero overhead when validation is disabled

**C++14+ (Enhanced functionality):**
- Optional compile-time validation for constant expressions
- `constexpr` evaluation of casts when possible
- Automatic fallback to runtime validation for non-constants
- Maintains full compatibility with C++11 behavior

**Feature detection:**
The library automatically detects your compiler's C++ standard support and enables appropriate features:

```cpp
#include <ncast/ncast.h>
using namespace ncast;

// These work in ALL C++ standards (C++11+):
int runtime_value = get_value();
auto result1 = numeric_cast<unsigned int>(runtime_value);  // Runtime validation

// These work in C++14+ ONLY:
constexpr int compile_time_value = 42;
constexpr auto result2 = numeric_cast<unsigned int>(compile_time_value);  // Compile-time validation
```

**Migration notes:**
- Existing C++11 code continues to work unchanged
- Enhanced features are automatically available when upgrading to C++14+
- No breaking changes between standards

### Configuration

```cpp
// Disable runtime validation globally for maximum performance
#define NCAST_DISABLE_RUNTIME_VALIDATION
#include <ncast/ncast.h>

// Disable compile-time validation (C++14+) while keeping runtime validation
#define NCAST_DISABLE_COMPILE_TIME_VALIDATION
#include <ncast/ncast.h>

// Disable both runtime and compile-time validation for ultimate performance
#define NCAST_DISABLE_RUNTIME_VALIDATION
#define NCAST_DISABLE_COMPILE_TIME_VALIDATION
#include <ncast/ncast.h>

// When runtime validation is disabled, all casts behave like static_cast but with type safety
// Runtime validation overhead: ~6.0% (see benchmarks)
// No-runtime-validation overhead: ~1-1.3% (nearly identical to static_cast)
```

**Control Macros:**

- **`NCAST_DISABLE_RUNTIME_VALIDATION`**: Disables runtime validation checks
  - Use for maximum performance when you're confident about the safety of your casts
  - Functions like `static_cast` but with compile-time type safety
  - Affects both C++11 and C++14+ modes

- **`NCAST_DISABLE_COMPILE_TIME_VALIDATION`**: Disables compile-time validation (C++14+ only)
  - Forces the use of runtime validation even for constant expressions
  - Useful for debugging or when you want consistent runtime behavior
  - Has no effect in C++11 mode (already runtime-only)

**Validation modes:**
- **With runtime validation** (default): Runtime checks with exceptions
- **Without runtime validation**: Compile-time type safety only, zero overhead
- **With compile-time validation** (C++14+, default): Compile-time validation for constant expressions
- **Without compile-time validation**: Forces runtime validation even for constants

## Examples

### Basic Usage

```cpp
#include <ncast/ncast.h>
using namespace ncast;

// Safe conversions
int a = numeric_cast<int>(42u);           // OK
unsigned int b = numeric_cast<unsigned int>(42);  // OK
char c = numeric_cast<char>(65);          // OK ('A')

// These would throw:
// unsigned int d = numeric_cast<unsigned int>(-1);     // Negative to unsigned
// char e = numeric_cast<char>(300);                    // Value too large
// int f = numeric_cast<int>(UINT_MAX);                 // Overflow
```

### Char Casting

```cpp
// Always safe between char types
signed char sc = -1;
unsigned char uc = char_cast<unsigned char>(sc);    // OK

char c = 'A';
signed char sc2 = char_cast<signed char>(c);        // OK

// Compile-time error:
// int bad = char_cast<int>(c);                      // Won't compile
```

### Compile-time Validation (C++14+)

```cpp
#include <ncast/ncast.h>
using namespace ncast;

// Compile-time numeric casting
constexpr int value = 42;
constexpr unsigned int result = numeric_cast<unsigned int>(value);  // Validated at compile time

// Compile-time char casting  
constexpr char c = 'A';
constexpr unsigned char uc = char_cast<unsigned char>(c);          // Evaluated at compile time

// Mixed compile-time and runtime
constexpr int compile_time_safe = numeric_cast<int>(100U);         // Compile-time
int runtime_value = get_input();
int runtime_safe = numeric_cast<int>(runtime_value);               // Runtime validation

// This will cause a compile error (intentional):
// constexpr auto bad = numeric_cast<unsigned int>(-1);            // Compile-time validation fails

// Feature detection
#if NCAST_HAS_CONSTEXPR_VALIDATION
    // C++14+ features available
    constexpr auto enhanced_result = numeric_cast<int>(42U);
#else
    // C++11 mode - runtime validation only
    auto basic_result = numeric_cast<int>(42U);
#endif
```

### Validation Control Examples

```cpp
// Example 1: Maximum performance - disable runtime validation
#define NCAST_DISABLE_RUNTIME_VALIDATION
#include <ncast/ncast.h>
using namespace ncast;

int unsafe_but_fast() {
    int negative = -1;
    // This will NOT throw even though it's unsafe (becomes large positive number)
    unsigned int result = numeric_cast<unsigned int>(negative);  // No runtime check
    return result;  // Returns 4294967295 (2^32 - 1)
}

// Example 2: Force runtime validation even for constants (C++14+)
#define NCAST_DISABLE_COMPILE_TIME_VALIDATION  
#include <ncast/ncast.h>
using namespace ncast;

void always_runtime_validation() {
    constexpr int value = 42;
    // Even though value is constexpr, this will use runtime validation
    auto result = numeric_cast<unsigned int>(value);  // Runtime validation
}

// Example 3: Ultimate performance - disable both validations
#define NCAST_DISABLE_RUNTIME_VALIDATION
#define NCAST_DISABLE_COMPILE_TIME_VALIDATION
#include <ncast/ncast.h>
using namespace ncast;

void maximum_performance() {
    // Behaves exactly like static_cast but with type safety
    auto result = numeric_cast<unsigned int>(-1);  // No validation at all
}
```

### Exception Handling

```cpp
try {
    int result = NUMERIC_CAST(int, some_large_value);
} catch (const ncast::cast_exception& e) {
    std::cout << "Cast failed: " << e.what() << std::endl;
    std::cout << "File: " << e.getFile() << std::endl;
    std::cout << "Line: " << e.getLine() << std::endl;
}
```

### Floating-Point Special Values

```cpp
#include <ncast/ncast.h>
#include <limits>
using namespace ncast;

// NaN handling
float f_nan = std::numeric_limits<float>::quiet_NaN();
double d_nan = numeric_cast<double>(f_nan);  // Works - NaN can be converted between floating types
// int i_nan = numeric_cast<int>(f_nan);     // Throws - NaN cannot be converted to integers

// Infinity handling
float f_inf = std::numeric_limits<float>::infinity();
double d_inf = numeric_cast<double>(f_inf);  // Works - infinity can be converted between floating types
// int i_inf = numeric_cast<int>(f_inf);     // Throws - infinity cannot be converted to integers

// Denormal values
double tiny = std::numeric_limits<double>::denorm_min();
try {
    float f_tiny = numeric_cast<float>(tiny);  // May throw if value is too small for float
} catch (const cast_exception& e) {
    // Handle denormal conversion failure
}
```

### Long Double Support

```cpp
// High-precision long double conversions with proper validation
#include <ncast/ncast.h>
#include <limits>
using namespace ncast;

// Basic long double conversions
long double ld = 123.456L;
double d = numeric_cast<double>(ld);        // OK: 123.456
float f = numeric_cast<float>(ld);          // OK: 123.456f

// Large integer to long double (preserves maximum precision)
unsigned long long big_int = 0x1FFFFFFFFFFFFF01ULL;
long double ld_result = numeric_cast<long double>(big_int);  // High precision preserved

// Range validation for long double
if (std::numeric_limits<long double>::max() > std::numeric_limits<double>::max()) {
    long double huge = std::numeric_limits<long double>::max();
    try {
        double d_overflow = numeric_cast<double>(huge);  // Throws: value too large
    } catch (const cast_exception& e) {
        // Properly detects long double overflow to smaller floating-point types
    }
}

// NaN and infinity handling with long double
long double ld_nan = std::numeric_limits<long double>::quiet_NaN();
double d_nan = numeric_cast<double>(ld_nan);     // OK: NaN between floating-point types
// int i_nan = numeric_cast<int>(ld_nan);        // Throws: NaN cannot convert to integer

long double ld_inf = std::numeric_limits<long double>::infinity();
// int i_inf = numeric_cast<int>(ld_inf);        // Throws: infinity cannot convert to integer
```

### Boolean Casting

```cpp
// Boolean conversions with validation
bool b1 = numeric_cast<bool>(0);    // OK: false
bool b2 = numeric_cast<bool>(1);    // OK: true
int i = numeric_cast<int>(true);    // OK: 1

// These would throw due to range validation:
// bool b3 = numeric_cast<bool>(2);     // Value too large for bool (max is 1)
// bool b4 = numeric_cast<bool>(-1);    // Negative value (bool is unsigned)
```

## Project Structure

```
ncast/
├── include/
│   ├── ncast/
│   │   └── ncast.h          # Main library header
│   └── utest/
│       └── utest.h          # Testing framework
├── tests/
│   ├── test_ncast_core.cpp     # Core functionality tests (basic casting, macros, integration)
│   ├── test_ncast_int.cpp      # Integer-specific tests (overflow, narrowing, size edge cases)
│   ├── test_ncast_float.cpp    # Floating-point tests (conversions, NaN/infinity, long double)
│   └── test_ncast_char.cpp     # Character-specific tests (char_cast, ASCII, boundaries)
├── demos/
│   ├── demo_ncast.cpp       # Usage demonstrations
│   └── benchmark_ncast.cpp  # Performance benchmarks
├── docs/
│   ├── Doxyfile.in          # Doxygen configuration
│   └── html/                # Generated documentation
├── cmake/
│   └── ncastConfig.cmake.in # CMake package config
├── CMakeLists.txt           # Build configuration
├── rebuild.sh               # Build from scratch
├── run_tests.sh             # Execute tests
├── run_demos.sh             # Execute demos
├── run_benchmarks.sh        # Execute benchmarks
├── build_docs.sh            # Generate documentation
└── README.md                # This file
```

## Testing

The library includes comprehensive tests organized into focused modules:

### Test Modules

- **`test_ncast_core`**: Core functionality tests
  - Basic casting operations between all numeric types
  - Macro functionality (`NUMERIC_CAST`, `CHAR_CAST`) and location information
  - Integration tests combining multiple conversion types
  
- **`test_ncast_int`**: Integer-specific tests
  - Overflow/underflow detection (signed↔unsigned, narrowing conversions)
  - Boundary value testing using `std::numeric_limits`
  - Platform-specific integer size handling
  - Sign conversion edge cases

- **`test_ncast_float`**: Floating-point tests
  - Float↔int conversions with proper truncation and range validation
  - Float↔double conversions including precision loss scenarios
  - Special value handling (NaN, infinity, signed zero, denormalized values)
  - Long double specific tests with high-precision validation
  - Extreme value and subnormal number handling

- **`test_ncast_char`**: Character-specific tests
  - `char_cast` functionality between char types
  - Int↔char conversions with ASCII range validation
  - Extended ASCII (128-255) and negative value handling
  - Boundary interactions between `char`, `signed char`, `unsigned char`

### Running Tests

**Individual test modules:**
```bash
cd build
./test_ncast_core     # Core functionality (5 tests)
./test_ncast_int      # Integer tests (4 tests)
./test_ncast_float    # Floating-point tests (15 tests)
./test_ncast_char     # Character tests (8 tests)
```

**All tests via CTest:**
```bash
./run_tests.sh              # Quick test run (all 4 modules)
cd build && ctest           # Run all test modules
cd build && ctest -V        # Verbose output
```

**Total test coverage**: 32 comprehensive tests across all modules covering every aspect of the library.

## Benchmarks

The library includes comprehensive performance benchmarks comparing:
- `static_cast` (baseline - fastest possible)
- `numeric_cast` function **without** validation (true no-validation, separate compilation unit)
- `numeric_cast` function **with** validation (runtime checks enabled)
- `NUMERIC_CAST` macro **without** validation (true no-validation, separate compilation unit)  
- `NUMERIC_CAST` macro **with** validation (runtime checks + location info)

**Advanced benchmark features:**
- Multi-module approach: No-validation tests compiled separately with `NCAST_DISABLE_RUNTIME_VALIDATION`
- Statistical analysis: Multiple runs with average, median, standard deviation, min, max
- Individual warm-up phases for each function to ensure fair comparison
- Heavy computational workload to measure real-world performance impact
- Configurable number of runs via command line parameter
- Comprehensive overhead analysis relative to static_cast baseline

**Sample benchmark results:**
```
=== Performance Summary (Average of 7 runs) ===
1. static_cast:                  1749.4 ms (±17.6)
2. numeric_cast (no validation):  1708.0 ms (±10.1)
3. numeric_cast (validation):    1854.6 ms (±25.2)
4. NUMERIC_CAST (no validation):  1736.5 ms (±23.3)
5. NUMERIC_CAST (validation):    1853.8 ms (±19.4)

Function validation overhead: 6.0%
Macro validation overhead:    6.0%
```

**Key insights:**
- No-validation versions perform very close to `static_cast`
- Validation overhead is moderate (6.0% for both functions and macros)
- Macro versions provide location info with minimal additional cost
- Perfect for performance-critical code when validation is disabled

**Run benchmarks:**
```bash
./run_benchmarks.sh             # Execute benchmarks with default settings (5 runs)
cd build && ./benchmark_ncast   # Direct execution with default parameters
cd build && ./benchmark_ncast 10  # Run with 10 iterations for better statistics
```

## Documentation

Generate comprehensive API documentation with Doxygen:
```bash
./build_docs.sh    # Generates docs/html/index.html
```

**Documentation includes:**
- Complete API reference with examples
- Function-by-function documentation
- Usage patterns and best practices
- Error conditions and exception details
- Performance considerations

**Online documentation:** [View Documentation](docs/html/index.html)

## Performance

- **With validation**: Runtime overhead for range checking (6.0%)
- **Without validation**: Minimal overhead (nearly identical to `static_cast`)
- **Header-only**: No linking required, enables maximum compiler optimizations
- **Benchmark-verified**: Comprehensive performance testing with real workloads

**Performance modes:**
```cpp
// High safety (default) - runtime validation enabled, compile-time validation for constants (C++14+)
#include <ncast/ncast.h>

// Maximum performance - runtime validation disabled, zero overhead
#define NCAST_DISABLE_RUNTIME_VALIDATION
#include <ncast/ncast.h>

// Force runtime validation even for constants (C++14+)
#define NCAST_DISABLE_COMPILE_TIME_VALIDATION
#include <ncast/ncast.h>
```

**Compiler optimization tips:**
- Use `-O3` or `-O2` for maximum performance
- Enable LTO (Link Time Optimization) for best results
- Profile your specific use case with the included benchmarks

## Contributing

We welcome contributions! Here's how you can help:

### Bug Reports
- Use GitHub Issues with a clear description
- Include minimal reproduction case
- Specify compiler, platform, and ncast version

### Feature Requests  
- Check existing issues first
- Describe the use case and expected behavior
- Consider performance implications

### Pull Requests
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Add tests for new functionality
4. Ensure all tests pass (`./run_tests.sh`)
5. Run benchmarks if performance-related (`./run_benchmarks.sh`)
6. Follow existing code style
7. Update documentation if needed
8. Commit with clear messages
9. Push and create a Pull Request

### Development Setup
```bash
git clone https://github.com/vpiotr/ncast.git
cd ncast
./rebuild.sh      # Build everything
./run_tests.sh    # Verify tests pass
./run_benchmarks.sh  # Check performance
```

### Coding Standards
- Follow existing naming conventions
- Add comprehensive tests for new features
- Document public APIs with Doxygen comments
- Ensure warning-free compilation
- Maintain header-only design

## License

MIT License

Copyright (c) 2025 Piotr Likus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Author

**ncast** is developed and maintained by:

- **Primary Author**: Piotr Likus ([@vpiotr](https://github.com/vpiotr))
- **GitHub**: [https://github.com/vpiotr/ncast](https://github.com/vpiotr/ncast)

### Acknowledgments

- Thanks to the C++ community for feedback and suggestions
- Inspired by the need for safer numeric conversions in modern C++
- Built with careful attention to performance and usability

### Support

- **Documentation**: [View Docs](docs/html/index.html)
- **Issues**: [GitHub Issues](https://github.com/vpiotr/ncast/issues)
- **Discussions**: [GitHub Discussions](https://github.com/vpiotr/ncast/discussions)

---

**Star this project if you find it helpful!**
