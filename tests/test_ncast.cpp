#include "../include/ncast/ncast.h"
#include "../include/utest/utest.h"
#include <climits>
#include <limits>

using namespace ncast;

// Test basic successful casts
UTEST_FUNC_DEF(NumberCastBasic) {
    // Test basic successful casts
    UTEST_ASSERT_EQUALS(42, number_cast<int>(42));
    UTEST_ASSERT_EQUALS(42u, number_cast<unsigned int>(42));
    UTEST_ASSERT_EQUALS(42.0f, number_cast<float>(42));
    UTEST_ASSERT_EQUALS(42.0, number_cast<double>(42.0f));
}

// Test char casting
UTEST_FUNC_DEF(NumberCastChar) {
    UTEST_ASSERT_EQUALS('A', number_cast<char>('A'));
    UTEST_ASSERT_EQUALS(65, number_cast<int>('A'));
    
    // Test char boundary values
    UTEST_ASSERT_EQUALS(static_cast<char>(127), number_cast<char>(127));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(255), number_cast<unsigned char>(255));
}

// Test overflow detection - signed to unsigned
UTEST_FUNC_DEF(NumberCastSignedToUnsigned) {
    // Negative values should throw when casting to unsigned
    UTEST_ASSERT_THROWS([](){ number_cast<unsigned int>(-1); });
    UTEST_ASSERT_THROWS([](){ number_cast<unsigned char>(-1); });
    UTEST_ASSERT_THROWS([](){ number_cast<unsigned short>(-42); });
    
    // Large positive values should work
    int positive_val = 1000;
    UTEST_ASSERT_EQUALS(1000u, number_cast<unsigned int>(positive_val));
}

// Test overflow detection - unsigned to signed
UTEST_FUNC_DEF(NumberCastUnsignedToSigned) {
    // Values that are too large for signed type should throw
    unsigned int large_val = static_cast<unsigned int>(std::numeric_limits<int>::max()) + 1u;
    UTEST_ASSERT_THROWS([large_val](){ number_cast<int>(large_val); });
    
    // Values within range should work
    unsigned int small_val = 1000u;
    UTEST_ASSERT_EQUALS(1000, number_cast<int>(small_val));
}

// Test boundary value casting
UTEST_FUNC_DEF(NumberCastBoundaryValues) {
    // Test maximum values
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), 
                        number_cast<int>(std::numeric_limits<int>::max()));
    
    // Test minimum values
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::min(), 
                        number_cast<int>(std::numeric_limits<int>::min()));
    
    // Test zero
    UTEST_ASSERT_EQUALS(0, number_cast<int>(0));
    UTEST_ASSERT_EQUALS(0u, number_cast<unsigned int>(0));
}

// Test narrowing conversions
UTEST_FUNC_DEF(NumberCastNarrowing) {
    // Large long to int should throw if it doesn't fit
    long large_long = static_cast<long>(std::numeric_limits<int>::max()) + 1;
    UTEST_ASSERT_THROWS([large_long](){ number_cast<int>(large_long); });
    
    // Small long to int should work
    long small_long = 42;
    UTEST_ASSERT_EQUALS(42, number_cast<int>(small_long));
}

// Test floating point to integer conversion
UTEST_FUNC_DEF(NumberCastFloatToInt) {
    // Normal conversion
    UTEST_ASSERT_EQUALS(42, number_cast<int>(42.0));
    UTEST_ASSERT_EQUALS(42, number_cast<int>(42.7)); // Should truncate
    
    // Large float that doesn't fit in int should throw
    float large_float = static_cast<float>(std::numeric_limits<int>::max()) * 2.0f;
    UTEST_ASSERT_THROWS([large_float](){ number_cast<int>(large_float); });
}

// Test char_cast function
UTEST_FUNC_DEF(CharCastBasic) {
    // Basic char casting
    char c1 = 'A';
    unsigned char uc1 = 65;
    signed char sc1 = 65;
    
    UTEST_ASSERT_EQUALS('A', char_cast<char>(uc1));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>('A'), char_cast<unsigned char>(c1));
    UTEST_ASSERT_EQUALS(static_cast<signed char>('A'), char_cast<signed char>(c1));
}

// Test char_cast boundary values
UTEST_FUNC_DEF(CharCastBoundary) {
    // Test boundary values for char types
    unsigned char max_uchar = 255;
    signed char max_schar = 127;
    signed char min_schar = -128;
    
    // These should work without throwing
    UTEST_ASSERT_EQUALS(static_cast<char>(127), char_cast<char>(max_schar));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(127), char_cast<unsigned char>(max_schar));
    
    // Negative signed char to unsigned char should work (reinterpret)
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(-1), char_cast<unsigned char>(static_cast<signed char>(-1)));
}

// Test macro versions
UTEST_FUNC_DEF(MacroVersions) {
    // Test NUMBER_CAST macro
    int value = 42;
    UTEST_ASSERT_EQUALS(42u, NUMBER_CAST(unsigned int, value));
    
    // Test CHAR_CAST macro
    char c = 'X';
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>('X'), CHAR_CAST(unsigned char, c));
}

// Test macro exception information
UTEST_FUNC_DEF(MacroExceptionInfo) {
    // Test that macro provides additional context in exception
    try {
        int negative = -1;
        NUMBER_CAST(unsigned int, negative);
        UTEST_ASSERT_TRUE(false); // Should not reach here
    } catch (const ncast::cast_exception& e) {
        std::string msg = e.what();
        // Should contain file and line information
        UTEST_ASSERT_STR_CONTAINS(msg, "test_ncast.cpp");
        UTEST_ASSERT_STR_CONTAINS(msg, "Line");
    }
}

int main() {
    UTEST_PROLOG();
    
    UTEST_FUNC(NumberCastBasic);
    UTEST_FUNC(NumberCastChar);
    UTEST_FUNC(NumberCastSignedToUnsigned);
    UTEST_FUNC(NumberCastUnsignedToSigned);
    UTEST_FUNC(NumberCastBoundaryValues);
    UTEST_FUNC(NumberCastNarrowing);
    UTEST_FUNC(NumberCastFloatToInt);
    UTEST_FUNC(CharCastBasic);
    UTEST_FUNC(CharCastBoundary);
    UTEST_FUNC(MacroVersions);
    UTEST_FUNC(MacroExceptionInfo);
    
    UTEST_EPILOG();
    
    return 0;
}
