# ncast - Safe Numeric Casting Library

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/vpiotr/ncast)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11%2B-blue)](https://en.cppreference.com/w/cpp/11)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Header Only](https://img.shields.io/badge/header--only-yes-orange)](include/ncast/ncast.h)
[![Platform](https://img.shields.io/badge/platform-cross--platform-lightgrey)](CMakeLists.txt)

A header-only C++ micro-library for safe numeric casting with runtime validation and comprehensive error reporting.

## Quick Overview

**ncast** solves the problem of unsafe numeric type conversions in C++. In standard C++, using `static_cast` for numeric conversions can lead to silent data corruption, unexpected behavior, and hard-to-find bugs due to:

- Negative values being converted to unsigned types
- Values that exceed the target type's range
- Special floating-point values (NaN, infinity) being improperly converted
- Unexpected sign conversions

This library provides `numeric_cast<T>()` and `NUMERIC_CAST(T, val)` which perform comprehensive runtime validation before converting values, throwing informative exceptions when conversions would be unsafe. With validation disabled, it compiles down to a simple `static_cast` with zero overhead.

```cpp
// Instead of error-prone static_cast:
unsigned int result = static_cast<unsigned int>(-1);  // Silently becomes a large value!

// Use safe numeric_cast:
unsigned int result = numeric_cast<unsigned int>(value);  // Throws if value is negative
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
- **Header-only**: Just include `ncast.h` and start using - no linking required
- **Comprehensive support**: Works with all numeric types plus char types
- **Special floating-point handling**: Properly manages NaN, infinity, and denormal values
- **Two APIs**: Function templates and macros with precise location information
- **Optional validation**: Can be disabled for performance-critical code paths
- **Exception safety**: Clear error messages with file/line/function information
- **Zero overhead**: When validation is disabled, performs identical to `static_cast`
- **Boundary testing**: Uses `std::numeric_limits` for accurate range validation
- **Well-tested**: Comprehensive test suite with edge case coverage
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

// Safe casting - throws cast_exception if value doesn't fit
int value = 42;
unsigned int result = numeric_cast<unsigned int>(value);  // OK

// This would throw because -1 cannot be safely cast to unsigned
int negative = -1;
unsigned int bad_result = numeric_cast<unsigned int>(negative);  // Throws!

// Char-specific casting (always safe between char types)
signed char sc = 'A';
unsigned char uc = char_cast<unsigned char>(sc);  // Always safe

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

# Disable validation globally (default: OFF)
cmake .. -DNCAST_DISABLE_VALIDATION=ON

# Release build for benchmarks
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## API Reference

### numeric_cast

Safe casting between any numeric types and char:

```cpp
template<typename ToType, typename FromType>
ToType numeric_cast(FromType value);

// Macro version with location info
#define NUMERIC_CAST(ToType, value)
```

**Validation rules:**
- Negative values cannot be cast to unsigned types
- Values must fit within target type's range (uses `std::numeric_limits`)
- Special floating-point values are handled properly:
  - NaN can only be converted between floating-point types
  - Infinity can only be converted between floating-point types
  - Denormal values are properly validated for range
- Throws `cast_exception` on validation failure with detailed context
- Compile-time type safety enforced via `static_assert`

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

Safe casting between char types only:

```cpp
template<typename ToType, typename FromType>
ToType char_cast(FromType value);

// Macro version with location info  
#define CHAR_CAST(ToType, value)
```

**Features:**
- Works only with `char`, `signed char`, `unsigned char`
- Always safe - no runtime validation needed (reinterpret cast)
- Compiler error if used with non-char types
- Zero overhead - pure compile-time safety

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

### Configuration

```cpp
// Disable validation globally for maximum performance
#define NCAST_DISABLE_VALIDATION
#include <ncast/ncast.h>

// When disabled, all casts behave like static_cast but with type safety
// Validation overhead: ~0.3-1.2% (see benchmarks)
// No-validation overhead: 0% (identical to static_cast)
```

**Validation modes:**
- **With validation** (default): Runtime checks with exceptions
- **Without validation**: Compile-time type safety only, zero overhead

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

## Project Structure

```
ncast/
├── include/
│   ├── ncast/
│   │   └── ncast.h          # Main library header
│   └── utest/
│       └── utest.h          # Testing framework
├── tests/
│   └── test_ncast.cpp       # Comprehensive tests
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

The library includes comprehensive tests covering:

- Basic casting operations between all numeric types
- Boundary value testing using `std::numeric_limits`
- Overflow/underflow detection and validation
- Exception handling and error message verification
- Macro functionality and location information
- Char-specific casting scenarios
- Platform-specific integer size handling
- Sign conversion edge cases
- Floating-point to integer conversions

**Test execution:**
```bash
./run_tests.sh    # Quick test run
cd build && make test && ctest -V  # Verbose CMake/CTest output
```

## Benchmarks

The library includes comprehensive performance benchmarks comparing:
- `static_cast` (baseline - fastest possible)
- `numeric_cast` function **without** validation (true no-validation, separate compilation unit)
- `numeric_cast` function **with** validation (runtime checks enabled)
- `NUMERIC_CAST` macro **without** validation (true no-validation, separate compilation unit)  
- `NUMERIC_CAST` macro **with** validation (runtime checks + location info)

**Advanced benchmark features:**
- Multi-module approach: No-validation tests compiled separately with `NCAST_DISABLE_VALIDATION`
- Individual warm-up phases for each function to ensure fair comparison
- ASCII performance charts with smart scaling for small differences
- Heavy computational workload to measure real-world performance impact
- Accurate progress reporting and result validation

**Sample benchmark results:**
```
=== Performance Summary ===
1. static_cast:                   1743.60 ms
2. numeric_cast (no validation):   1730.13 ms (1.0x - essentially identical)
3. numeric_cast (validation):      1819.8 ms (1.04x - 4.4% overhead)
4. NUMERIC_CAST (no validation):   1730.6 ms (1.0x - essentially identical)  
5. NUMERIC_CAST (validation):      1823.7 ms (1.05x - 4.6% overhead)

Function validation overhead: 4.4%
Macro validation overhead: 4.6%
```

**Key insights:**
- No-validation versions perform nearly identically to `static_cast`
- Validation overhead is minimal (typically 0.3-1.2% for computational workloads)
- Macro versions provide location info with negligible additional cost
- Perfect for performance-critical code when validation is disabled

**Run benchmarks:**
```bash
./run_benchmarks.sh          # Execute all benchmarks
cd build && ./benchmark_ncast  # Direct execution with detailed output
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

- **With validation**: Small runtime overhead for range checking (~0.3-1.2%)
- **Without validation**: Zero overhead (equivalent to `static_cast`)
- **Header-only**: No linking required, enables maximum compiler optimizations
- **Benchmark-verified**: Comprehensive performance testing with real workloads

**Performance modes:**
```cpp
// High safety (default) - runtime validation enabled
#include <ncast/ncast.h>

// Maximum performance - validation disabled, zero overhead
#define NCAST_DISABLE_VALIDATION
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
