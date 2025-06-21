#include <iostream>
#include <limits>
#include "../include/ncast/ncast.h"

using namespace ncast;

void demo_basic_usage() {
    std::cout << "=== Basic Usage Demo ===" << std::endl;
    
    // Basic successful casts
    int positive = 42;
    unsigned int result1 = number_cast<unsigned int>(positive);
    std::cout << "number_cast<unsigned int>(" << positive << ") = " << result1 << std::endl;
    
    // Using macro version
    unsigned int result2 = NUMBER_CAST(unsigned int, positive);
    std::cout << "NUMBER_CAST(unsigned int, " << positive << ") = " << result2 << std::endl;
    
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
        unsigned int result = number_cast<unsigned int>(negative);
        (void)result; // Suppress unused variable warning
        std::cout << "ERROR: This should not execute!" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    
    // Demonstrate overflow protection
    try {
        int big_value = 300;
        char result = number_cast<char>(big_value);
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
        char result = number_cast<char>(max_char);
        std::cout << "number_cast<char>(" << max_char << ") = " 
                  << static_cast<int>(result) << " (at limit)" << std::endl;
    } catch (const cast_exception& e) {
        std::cout << "Exception at limit: " << e.what() << std::endl;
    }
    
    try {
        int beyond_limit = static_cast<int>(std::numeric_limits<char>::max()) + 1;
        char result = number_cast<char>(beyond_limit);
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
        unsigned int result = NUMBER_CAST(unsigned int, negative);  // This line will be in error
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

int main() {
    std::cout << "ncast Library Demonstration" << std::endl;
    std::cout << "===========================" << std::endl << std::endl;
    
    demo_basic_usage();
    demo_safe_failures();
    demo_char_casting();
    demo_limits();
    demo_macro_location_info();
    
    std::cout << "Demo completed successfully!" << std::endl;
    
    return 0;
}
