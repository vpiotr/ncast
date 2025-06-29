#include "include/ncast/ncast.h"
#include <iostream>

int main() {
    using namespace ncast;
    
    // Test basic functionality that should work in all C++ standards
    int value1 = 42;
    unsigned int result1 = numeric_cast<unsigned int>(value1);
    std::cout << "Basic cast: " << value1 << " -> " << result1 << " (Success)" << std::endl;
    
    // Test char casting
    char c = 'A';
    unsigned char uc = char_cast<unsigned char>(c);
    std::cout << "Char cast: '" << c << "' -> " << static_cast<int>(uc) << " (Success)" << std::endl;
    
    // Print feature detection results
    std::cout << "\n=== Feature Detection ===" << std::endl;
    std::cout << "C++11 base functionality: Always available" << std::endl;
    
#if NCAST_HAS_CPP14
    std::cout << "C++14 features: Available" << std::endl;
#else
    std::cout << "C++14 features: Not available" << std::endl;
#endif

#if NCAST_HAS_CONSTEXPR_VALIDATION
    std::cout << "Constexpr validation: Available" << std::endl;
    
    // Test compile-time evaluation (C++14+ only)
    constexpr int compile_time_value = 42;
    constexpr auto compile_time_result = numeric_cast<unsigned int>(compile_time_value);
    std::cout << "Compile-time cast: " << compile_time_value << " -> " << compile_time_result << " (Success)" << std::endl;
    
    constexpr auto char_compile_time = char_cast<unsigned char>('B');
    std::cout << "Compile-time char cast: 'B' -> " << static_cast<int>(char_compile_time) << " (Success)" << std::endl;
#else
    std::cout << "Constexpr validation: Not available (using runtime validation)" << std::endl;
#endif

    // Test macro versions
    auto macro_result = NUMERIC_CAST(int, 100U);
    auto char_macro_result = CHAR_CAST(char, static_cast<unsigned char>(65));
    std::cout << "\nMacro versions work: " << macro_result << ", '" << char_macro_result << "'" << std::endl;
    
    // Test that runtime validation still works
    try {
        auto bad_result = numeric_cast<unsigned int>(-1);
        std::cout << "ERROR: Should have thrown!" << std::endl;
        return 1;
    } catch (const cast_exception& e) {
        std::cout << "Runtime validation works: Caught expected exception" << std::endl;
    }
    
    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
