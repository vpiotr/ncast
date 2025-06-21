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
    
    // Test char boundary values using numeric_limits
    UTEST_ASSERT_EQUALS(std::numeric_limits<char>::max(), 
                        number_cast<char>(std::numeric_limits<char>::max()));
    UTEST_ASSERT_EQUALS(std::numeric_limits<char>::min(), 
                        number_cast<char>(std::numeric_limits<char>::min()));
    UTEST_ASSERT_EQUALS(std::numeric_limits<unsigned char>::max(), 
                        number_cast<unsigned char>(std::numeric_limits<unsigned char>::max()));
}

// Test overflow detection - signed to unsigned
UTEST_FUNC_DEF(NumberCastSignedToUnsigned) {
    // Negative values should throw when casting to unsigned
    UTEST_ASSERT_THROWS([](){ number_cast<unsigned int>(-1); });
    UTEST_ASSERT_THROWS([](){ number_cast<unsigned char>(-1); });
    UTEST_ASSERT_THROWS([](){ number_cast<unsigned short>(-42); });
    
    // Test edge case: most negative value
    UTEST_ASSERT_THROWS([](){ 
        number_cast<unsigned int>(std::numeric_limits<int>::min()); 
    });
    
    // Large positive values should work
    int positive_val = 1000;
    UTEST_ASSERT_EQUALS(1000u, number_cast<unsigned int>(positive_val));
    
    // Test maximum signed value to unsigned (should work)
    UTEST_ASSERT_EQUALS(static_cast<unsigned int>(std::numeric_limits<int>::max()), 
                        number_cast<unsigned int>(std::numeric_limits<int>::max()));
}

// Test overflow detection - unsigned to signed
UTEST_FUNC_DEF(NumberCastUnsignedToSigned) {
    // Values that are too large for signed type should throw
    unsigned int large_val = static_cast<unsigned int>(std::numeric_limits<int>::max()) + 1u;
    UTEST_ASSERT_THROWS([large_val](){ number_cast<int>(large_val); });
    
    // Test maximum unsigned value (should throw when casting to signed)
    UTEST_ASSERT_THROWS([](){ 
        number_cast<int>(std::numeric_limits<unsigned int>::max()); 
    });
    
    // Test unsigned value that exactly equals signed max (should work)
    unsigned int max_signed_as_unsigned = static_cast<unsigned int>(std::numeric_limits<int>::max());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), 
                        number_cast<int>(max_signed_as_unsigned));
    
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
    
    // Test maximum long value (should throw when casting to int if long > int)
    if (std::numeric_limits<long>::max() > std::numeric_limits<int>::max()) {
        UTEST_ASSERT_THROWS([](){ 
            number_cast<int>(std::numeric_limits<long>::max()); 
        });
    }
    
    // Test minimum long value (should throw when casting to int if long < int)
    if (std::numeric_limits<long>::min() < std::numeric_limits<int>::min()) {
        UTEST_ASSERT_THROWS([](){ 
            number_cast<int>(std::numeric_limits<long>::min()); 
        });
    }
    
    // Small long to int should work
    long small_long = 42;
    UTEST_ASSERT_EQUALS(42, number_cast<int>(small_long));
    
    // Test casting within valid range
    long valid_long = std::numeric_limits<int>::max();
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), number_cast<int>(valid_long));
}

// Test floating point to integer conversion
UTEST_FUNC_DEF(NumberCastFloatToInt) {
    // Normal conversion
    UTEST_ASSERT_EQUALS(42, number_cast<int>(42.0));
    UTEST_ASSERT_EQUALS(42, number_cast<int>(42.7)); // Should truncate
    
    // Large float that doesn't fit in int should throw
    float large_float = static_cast<float>(std::numeric_limits<int>::max()) * 2.0f;
    UTEST_ASSERT_THROWS([large_float](){ number_cast<int>(large_float); });
    
    // Test edge cases with numeric_limits
    double max_int_as_double = static_cast<double>(std::numeric_limits<int>::max());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), number_cast<int>(max_int_as_double));
    
    double min_int_as_double = static_cast<double>(std::numeric_limits<int>::min());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::min(), number_cast<int>(min_int_as_double));
    
    // Test float/double values that exceed int range
    if (std::numeric_limits<double>::max() > std::numeric_limits<int>::max()) {
        UTEST_ASSERT_THROWS([](){ 
            number_cast<int>(std::numeric_limits<double>::max()); 
        });
    }
    
    if (std::numeric_limits<double>::lowest() < std::numeric_limits<int>::min()) {
        UTEST_ASSERT_THROWS([](){ 
            number_cast<int>(std::numeric_limits<double>::lowest()); 
        });
    }
}

// Test char_cast function
UTEST_FUNC_DEF(CharCastBasic) {
    // Basic char casting
    char c1 = 'A';
    unsigned char uc1 = 65;
    
    UTEST_ASSERT_EQUALS('A', char_cast<char>(uc1));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>('A'), char_cast<unsigned char>(c1));
    UTEST_ASSERT_EQUALS(static_cast<signed char>('A'), char_cast<signed char>(c1));
}

// Test char_cast boundary values
UTEST_FUNC_DEF(CharCastBoundary) {
    // Test boundary values for char types using numeric_limits
    unsigned char max_uchar = std::numeric_limits<unsigned char>::max();
    signed char max_schar = std::numeric_limits<signed char>::max();
    signed char min_schar = std::numeric_limits<signed char>::min();
    
    // These should work without throwing (char_cast is always safe between char types)
    UTEST_ASSERT_EQUALS(static_cast<char>(max_schar), char_cast<char>(max_schar));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(max_schar), char_cast<unsigned char>(max_schar));
    
    // Test maximum unsigned char to other char types
    UTEST_ASSERT_EQUALS(static_cast<char>(max_uchar), char_cast<char>(max_uchar));
    UTEST_ASSERT_EQUALS(static_cast<signed char>(max_uchar), char_cast<signed char>(max_uchar));
    
    // Test minimum signed char to other char types
    UTEST_ASSERT_EQUALS(static_cast<char>(min_schar), char_cast<char>(min_schar));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(min_schar), char_cast<unsigned char>(min_schar));
    
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

// Test edge cases with different integer sizes
UTEST_FUNC_DEF(NumberCastIntegerSizes) {
    // Test short to char (may overflow)
    if (std::numeric_limits<short>::max() > std::numeric_limits<char>::max()) {
        short large_short = std::numeric_limits<short>::max();
        UTEST_ASSERT_THROWS([large_short](){ number_cast<char>(large_short); });
    }
    
    // Test char to short (should always work since short >= char in range)
    char max_char = std::numeric_limits<char>::max();
    UTEST_ASSERT_EQUALS(static_cast<short>(max_char), number_cast<short>(max_char));
    
    // Test unsigned char to signed char (may overflow for values > 127)
    if (std::numeric_limits<unsigned char>::max() > std::numeric_limits<signed char>::max()) {
        unsigned char large_uchar = std::numeric_limits<unsigned char>::max();
        UTEST_ASSERT_THROWS([large_uchar](){ number_cast<signed char>(large_uchar); });
    }
    
    // Test within valid ranges
    unsigned char valid_uchar = static_cast<unsigned char>(std::numeric_limits<signed char>::max());
    UTEST_ASSERT_EQUALS(std::numeric_limits<signed char>::max(), 
                        number_cast<signed char>(valid_uchar));
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
    UTEST_FUNC(NumberCastIntegerSizes);
    
    UTEST_EPILOG();
    
    return 0;
}
