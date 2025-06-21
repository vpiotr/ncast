#include <iostream>
#include <limits>
#include "../include/ncast/ncast.h"

using namespace ncast;

void demo_basic_usage() {
    std::cout << "=== Basic Usage Demo ===" << std::endl;
    
    // Basic successful casts
    int positive = 42;
    unsigned int result1 = numeric_cast<unsigned int>(positive);
    std::cout << "numeric_cast<unsigned int>(" << positive << ") = " << result1 << std::endl;
    
    // Using macro version
    unsigned int result2 = NUMERIC_CAST(unsigned int, positive);
    std::cout << "NUMERIC_CAST(unsigned int, " << positive << ") = " << result2 << std::endl;
    
    // Char casting
    char c = 'A';
    unsigned char uc = char_cast<unsigned char>(c);
    std::cout << "char_cast<unsigned char>('" << c << "') = " << static_cast<int>(uc) << std::endl;
    
    std::cout << std::endl;
}

void demo_safe_failures() {
    std::cout << "=== Safe Failure Demo ===" << std::endl;
    
    // Demonstrate safe failure on negative to unsigned
    try {
        int negative = -42;
        unsigned int result = numeric_cast<unsigned int>(negative);
        (void)result; // Suppress unused variable warning
        std::cout << "ERROR: This should not execute!" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    
    // Demonstrate overflow protection
    try {
        int big_value = 300;
        char result = numeric_cast<char>(big_value);
        (void)result; // Suppress unused variable warning
        std::cout << "ERROR: This should not execute!" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

void demo_char_casting() {
    std::cout << "=== Char Casting Demo ===" << std::endl;
    
    // Show that char_cast is always safe between char types
    signed char sc = -1;
    unsigned char uc1 = char_cast<unsigned char>(sc);
    std::cout << "char_cast<unsigned char>(" << static_cast<int>(sc) << ") = " 
              << static_cast<int>(uc1) << " (safe conversion)" << std::endl;
    
    unsigned char uc = 255;
    signed char sc1 = char_cast<signed char>(uc);
    std::cout << "char_cast<signed char>(" << static_cast<int>(uc) << ") = " 
              << static_cast<int>(sc1) << " (safe conversion)" << std::endl;
    
    std::cout << std::endl;
}

void demo_limits() {
    std::cout << "=== Limits Demo ===" << std::endl;
    
    // Show casting at the limits
    try {
        int max_char = std::numeric_limits<char>::max();
        char result = numeric_cast<char>(max_char);
        std::cout << "numeric_cast<char>(" << max_char << ") = " 
                  << static_cast<int>(result) << " (at limit)" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Exception at limit: " << e.what() << std::endl;
    }
    
    try {
        int beyond_limit = static_cast<int>(std::numeric_limits<char>::max()) + 1;
        char result = numeric_cast<char>(beyond_limit);
        (void)result; // Suppress unused variable warning
        std::cout << "ERROR: This should not execute!" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Caught expected exception for beyond limit: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

void demo_macro_location_info() {
    std::cout << "=== Macro Location Info Demo ===" << std::endl;
    
    try {
        int negative = -123;
        unsigned int result = NUMERIC_CAST(unsigned int, negative);  // This line will be in error
        (void)result; // Suppress unused variable warning
        std::cout << "ERROR: This should not execute!" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Exception with location info:" << std::endl;
        std::cout << "  " << e.what() << std::endl;
        std::cout << "  File: " << e.getFile() << std::endl;
        std::cout << "  Line: " << e.getLine() << std::endl;
        std::cout << "  Function: " << e.getFunction() << std::endl;
    }
    
    std::cout << std::endl;
}

void demo_float_conversions() {
    std::cout << "=== Floating Point Conversion Demo ===" << std::endl;
    
    // Integer to float conversions
    int i = 42;
    float f = numeric_cast<float>(i);
    double d = numeric_cast<double>(i);
    std::cout << "numeric_cast<float>(" << i << ") = " << f << std::endl;
    std::cout << "numeric_cast<double>(" << i << ") = " << d << std::endl;
    
    // Float to double (always safe)
    float f2 = 3.14159f;
    double d2 = numeric_cast<double>(f2);
    std::cout << "numeric_cast<double>(" << f2 << "f) = " << d2 << std::endl;
    
    // Double to float (may lose precision but still valid)
    double d3 = 3.14159265358979323846;
    float f3 = numeric_cast<float>(d3);
    std::cout << "numeric_cast<float>(" << d3 << ") = " << f3 << " (precision loss but valid)" << std::endl;
    
    // Demonstrate range protection
    try {
        double too_large = static_cast<double>(std::numeric_limits<float>::max()) * 2.0;
        float result = numeric_cast<float>(too_large);
        (void)result; // Suppress unused variable warning
        std::cout << "ERROR: This should not execute!" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    std::cout << "ncast Library Demonstration" << std::endl;
    std::cout << "===========================" << std::endl << std::endl;
    
    demo_basic_usage();
    demo_safe_failures();
    demo_char_casting();
    demo_limits();
    demo_macro_location_info();
    demo_float_conversions();
    
    std::cout << "Demo completed successfully!" << std::endl;
    
    return 0;
}
