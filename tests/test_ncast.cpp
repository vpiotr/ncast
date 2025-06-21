#include "../include/ncast/ncast.h"
#include "../include/utest/utest.h"
#include <climits>
#include <limits>
#include <cmath> // For std::isnan, std::isinf, std::pow

using namespace ncast;

// =============================================================================
// BASIC FUNCTIONALITY TESTS
// =============================================================================

// Test basic successful casts between similar types
UTEST_FUNC_DEF(NumericCastBasic) {
    // Same type casting
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42));
    UTEST_ASSERT_EQUALS(42u, numeric_cast<unsigned int>(42u));
    
    // Widening conversions (always safe)
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(static_cast<short>(42)));
    UTEST_ASSERT_EQUALS(42.0f, numeric_cast<float>(42));
    UTEST_ASSERT_EQUALS(42.0, numeric_cast<double>(42.0f));
    
    // Sign conversion without overflow
    UTEST_ASSERT_EQUALS(42u, numeric_cast<unsigned int>(42));
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42u));
}

// Test basic char operations
UTEST_FUNC_DEF(NumericCastCharBasic) {
    // Basic char to int and back
    UTEST_ASSERT_EQUALS('A', numeric_cast<char>('A'));
    UTEST_ASSERT_EQUALS(65, numeric_cast<int>('A'));
    UTEST_ASSERT_EQUALS('B', numeric_cast<char>(66));
    
    // Basic char type conversions
    char c = 'X';
    unsigned char uc = 88;
    signed char sc = 'Y';
    
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>('X'), numeric_cast<unsigned char>(c));
    UTEST_ASSERT_EQUALS(88, numeric_cast<int>(uc));
    UTEST_ASSERT_EQUALS('Y', numeric_cast<char>(sc));
}

// Test signed to unsigned overflow detection
UTEST_FUNC_DEF(SignedToUnsignedOverflow) {
    // Negative values should always throw when casting to unsigned
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned int>(-1); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned char>(-1); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned short>(-42); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned long>(-100); });
    
    // Test edge case: most negative value
    UTEST_ASSERT_THROWS([](){ 
        numeric_cast<unsigned int>(std::numeric_limits<int>::min()); 
    });
    
    // Large positive values should work if they fit
    int positive_val = 1000;
    UTEST_ASSERT_EQUALS(1000u, numeric_cast<unsigned int>(positive_val));
    
    // Test maximum signed value to unsigned (should always work)
    UTEST_ASSERT_EQUALS(static_cast<unsigned int>(std::numeric_limits<int>::max()), 
                        numeric_cast<unsigned int>(std::numeric_limits<int>::max()));
}

// Test unsigned to signed overflow detection  
UTEST_FUNC_DEF(UnsignedToSignedOverflow) {
    // Values that exceed signed type maximum should throw
    unsigned int large_val = static_cast<unsigned int>(std::numeric_limits<int>::max()) + 1u;
    UTEST_ASSERT_THROWS([large_val](){ numeric_cast<int>(large_val); });
    
    // Test maximum unsigned value (should throw when casting to signed)
    UTEST_ASSERT_THROWS([](){ 
        numeric_cast<int>(std::numeric_limits<unsigned int>::max()); 
    });
    
    // Test unsigned value that exactly equals signed max (should work)
    unsigned int max_signed_as_unsigned = static_cast<unsigned int>(std::numeric_limits<int>::max());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), 
                        numeric_cast<int>(max_signed_as_unsigned));
    
    // Values within signed range should work
    unsigned int small_val = 1000u;
    UTEST_ASSERT_EQUALS(1000, numeric_cast<int>(small_val));
}

// Test narrowing conversions that may overflow
UTEST_FUNC_DEF(NarrowingConversions) {
    // Large long to int should throw if it doesn't fit
    if (std::numeric_limits<long>::max() > std::numeric_limits<int>::max()) {
        long large_long = static_cast<long>(std::numeric_limits<int>::max()) + 1;
        UTEST_ASSERT_THROWS([large_long](){ numeric_cast<int>(large_long); });
        
        // Test maximum long value 
        UTEST_ASSERT_THROWS([](){ 
            numeric_cast<int>(std::numeric_limits<long>::max()); 
        });
    }
    
    // Test minimum long value 
    if (std::numeric_limits<long>::min() < std::numeric_limits<int>::min()) {
        UTEST_ASSERT_THROWS([](){ 
            numeric_cast<int>(std::numeric_limits<long>::min()); 
        });
    }
    
    // Values within target range should work
    long small_long = 42;
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(small_long));
    
    long valid_long = std::numeric_limits<int>::max();
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), numeric_cast<int>(valid_long));
}

// Test integer size edge cases
UTEST_FUNC_DEF(IntegerSizeEdgeCases) {
    // Test short to char (may overflow)
    if (std::numeric_limits<short>::max() > std::numeric_limits<char>::max()) {
        short large_short = std::numeric_limits<short>::max();
        UTEST_ASSERT_THROWS([large_short](){ numeric_cast<char>(large_short); });
    }
    
    // Test char to short (should always work since short >= char in range)
    char max_char = std::numeric_limits<char>::max();
    UTEST_ASSERT_EQUALS(static_cast<short>(max_char), numeric_cast<short>(max_char));
    
    // Test unsigned char to signed char (may overflow for values > 127)
    if (std::numeric_limits<unsigned char>::max() > std::numeric_limits<signed char>::max()) {
        unsigned char large_uchar = std::numeric_limits<unsigned char>::max();
        UTEST_ASSERT_THROWS([large_uchar](){ numeric_cast<signed char>(large_uchar); });
    }
    
    // Test within valid ranges
    unsigned char valid_uchar = static_cast<unsigned char>(std::numeric_limits<signed char>::max());
    UTEST_ASSERT_EQUALS(std::numeric_limits<signed char>::max(), 
                        numeric_cast<signed char>(valid_uchar));
}

// =============================================================================
// FLOATING POINT CONVERSION TESTS
// =============================================================================

// Test integer to floating point conversion
UTEST_FUNC_DEF(IntToFloatConversion) {
    // Basic integer to float/double conversions
    UTEST_ASSERT_EQUALS(42.0f, numeric_cast<float>(42));
    UTEST_ASSERT_EQUALS(42.0, numeric_cast<double>(42));
    UTEST_ASSERT_EQUALS(-42.0f, numeric_cast<float>(-42));
    UTEST_ASSERT_EQUALS(-42.0, numeric_cast<double>(-42));
    
    // Unsigned integer to float/double
    UTEST_ASSERT_EQUALS(42.0f, numeric_cast<float>(42u));
    UTEST_ASSERT_EQUALS(42.0, numeric_cast<double>(42u));
    
    // Large integer values (that can be represented exactly in floating point)
    UTEST_ASSERT_EQUALS(1000000.0f, numeric_cast<float>(1000000));
    UTEST_ASSERT_EQUALS(1000000.0, numeric_cast<double>(1000000));
    
    // Test maximum integer values
    int max_int = std::numeric_limits<int>::max();
    UTEST_ASSERT_EQUALS(static_cast<float>(max_int), numeric_cast<float>(max_int));
    UTEST_ASSERT_EQUALS(static_cast<double>(max_int), numeric_cast<double>(max_int));
    
    // Test minimum integer values
    int min_int = std::numeric_limits<int>::min();
    UTEST_ASSERT_EQUALS(static_cast<float>(min_int), numeric_cast<float>(min_int));
    UTEST_ASSERT_EQUALS(static_cast<double>(min_int), numeric_cast<double>(min_int));
    
    // Test large unsigned int (potentially larger than max int)
    unsigned int large_uint = std::numeric_limits<unsigned int>::max();
    UTEST_ASSERT_EQUALS(static_cast<float>(large_uint), numeric_cast<float>(large_uint));
    UTEST_ASSERT_EQUALS(static_cast<double>(large_uint), numeric_cast<double>(large_uint));
    
    // Test precision loss for large integers to float
    // For integers larger than 2^24, float may lose precision
    if (sizeof(long) * 8 > 24) {
        long large_precise_int = (1L << 24) - 1; // 2^24 - 1, can be represented exactly
        UTEST_ASSERT_EQUALS(static_cast<float>(large_precise_int), numeric_cast<float>(large_precise_int));
        
        long large_imprecise_int = (1L << 24) + 1; // 2^24 + 1, will lose precision in float
        // This should still work because we're just checking range, not precision
        UTEST_ASSERT_EQUALS(static_cast<float>(large_imprecise_int), numeric_cast<float>(large_imprecise_int));
    }
}

// Test floating point to integer conversion
UTEST_FUNC_DEF(FloatToIntConversion) {
    // Normal conversions with truncation
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42.0));
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42.7)); // Should truncate
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42.9)); // Should truncate
    UTEST_ASSERT_EQUALS(-42, numeric_cast<int>(-42.7)); // Should truncate toward zero
    
    // Test exact boundary values
    double max_int_as_double = static_cast<double>(std::numeric_limits<int>::max());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), numeric_cast<int>(max_int_as_double));
    
    double min_int_as_double = static_cast<double>(std::numeric_limits<int>::min());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::min(), numeric_cast<int>(min_int_as_double));
    
    // Values that exceed int range should throw
    if (std::numeric_limits<double>::max() > std::numeric_limits<int>::max()) {
        double large_double = static_cast<double>(std::numeric_limits<int>::max()) * 2.0;
        UTEST_ASSERT_THROWS([large_double](){ numeric_cast<int>(large_double); });
        
        UTEST_ASSERT_THROWS([](){ 
            numeric_cast<int>(std::numeric_limits<double>::max()); 
        });
    }
    
    if (std::numeric_limits<double>::lowest() < std::numeric_limits<int>::min()) {
        UTEST_ASSERT_THROWS([](){ 
            numeric_cast<int>(std::numeric_limits<double>::lowest()); 
        });
    }
    
    // Test float edge cases
    float large_float = static_cast<float>(std::numeric_limits<int>::max()) * 2.0f;
    UTEST_ASSERT_THROWS([large_float](){ numeric_cast<int>(large_float); });
}

// Test floating point to floating point conversion
UTEST_FUNC_DEF(FloatToDoubleConversion) {
    // Simple float to double (always safe, no precision loss)
    float f1 = 42.5f;
    UTEST_ASSERT_EQUALS(42.5, numeric_cast<double>(f1));
    
    // Double to float (may lose precision but should succeed if in range)
    double d1 = 42.5;
    UTEST_ASSERT_EQUALS(42.5f, numeric_cast<float>(d1));
    
    // Maximum and minimum values
    float max_float = std::numeric_limits<float>::max();
    UTEST_ASSERT_EQUALS(static_cast<double>(max_float), numeric_cast<double>(max_float));
    
    float lowest_float = std::numeric_limits<float>::lowest();
    UTEST_ASSERT_EQUALS(static_cast<double>(lowest_float), numeric_cast<double>(lowest_float));
    
    // Double to float overflow/underflow tests
    double large_double = std::numeric_limits<double>::max();
    if (large_double > static_cast<double>(std::numeric_limits<float>::max())) {
        UTEST_ASSERT_THROWS([large_double](){ numeric_cast<float>(large_double); });
    }
    
    double small_double = std::numeric_limits<double>::lowest();
    if (small_double < static_cast<double>(std::numeric_limits<float>::lowest())) {
        UTEST_ASSERT_THROWS([small_double](){ numeric_cast<float>(small_double); });
    }
    
    // Normal values near the boundaries
    double near_max_float = static_cast<double>(std::numeric_limits<float>::max()) * 0.9;
    UTEST_ASSERT_EQUALS(static_cast<float>(near_max_float), numeric_cast<float>(near_max_float));
    
    double near_min_float = static_cast<double>(std::numeric_limits<float>::lowest()) * 0.9;
    UTEST_ASSERT_EQUALS(static_cast<float>(near_min_float), numeric_cast<float>(near_min_float));
    
    // Small denormalized values
    double small_denorm = std::numeric_limits<double>::denorm_min();
    
    // Now handle denormalized values
    // Some implementations may accept denormalized values that are smaller than float's minimum
    // Others may reject them. Both behaviors are acceptable.
    try {
        float result = numeric_cast<float>(small_denorm);
        // If it succeeds, make sure the result is valid
        if (small_denorm >= static_cast<double>(std::numeric_limits<float>::denorm_min())) {
            UTEST_ASSERT_EQUALS(static_cast<float>(small_denorm), result);
        }
    } catch (const cast_exception& e) {
        // This is acceptable if the denorm value is too small for float
        // No further assertion needed
    }
    
    // Precision loss tests - values that will lose precision when converted to float
    double precise_double = 1.0 + std::pow(2.0, -24); // Just beyond float precision
    // This should still succeed because we check range, not precision
    UTEST_ASSERT_EQUALS(static_cast<float>(precise_double), numeric_cast<float>(precise_double));
}

// Test double to float conversion specifically
UTEST_FUNC_DEF(DoubleToFloatConversion) {
    // Basic conversions that maintain precision
    double d1 = 42.5;
    UTEST_ASSERT_EQUALS(42.5f, numeric_cast<float>(d1));
    
    double d2 = -123.75;
    UTEST_ASSERT_EQUALS(-123.75f, numeric_cast<float>(d2));
    
    // Range boundary tests
    // Values just within float range (should work)
    double near_max_float = static_cast<double>(std::numeric_limits<float>::max()) * 0.99;
    UTEST_ASSERT_EQUALS(static_cast<float>(near_max_float), numeric_cast<float>(near_max_float));
    
    double near_min_float = static_cast<double>(std::numeric_limits<float>::lowest()) * 0.99;
    UTEST_ASSERT_EQUALS(static_cast<float>(near_min_float), numeric_cast<float>(near_min_float));
    
    // Values outside float range (should throw)
    double beyond_max_float = static_cast<double>(std::numeric_limits<float>::max()) * 1.01;
    UTEST_ASSERT_THROWS([beyond_max_float](){ numeric_cast<float>(beyond_max_float); });
    
    double beyond_min_float = static_cast<double>(std::numeric_limits<float>::lowest()) * 1.01;
    UTEST_ASSERT_THROWS([beyond_min_float](){ numeric_cast<float>(beyond_min_float); });
    
    // Extreme values
    double max_double = std::numeric_limits<double>::max();
    UTEST_ASSERT_THROWS([max_double](){ numeric_cast<float>(max_double); });
    
    double min_double = std::numeric_limits<double>::lowest();
    UTEST_ASSERT_THROWS([min_double](){ numeric_cast<float>(min_double); });
    
    // Denormalized values
    // Double denormalized value that is representable in float
    double d_denorm1 = std::numeric_limits<float>::denorm_min() * 2.0;
    UTEST_ASSERT_EQUALS(static_cast<float>(d_denorm1), numeric_cast<float>(d_denorm1));
    
    // Double denormalized value that is too small for float
    double d_denorm2 = std::numeric_limits<float>::denorm_min() * 0.5;
    try {
        float result = numeric_cast<float>(d_denorm2);
        // If conversion succeeded, the result should be either 0.0f or denorm_min
        UTEST_ASSERT_TRUE(result == 0.0f || result == std::numeric_limits<float>::denorm_min());
    } catch (const cast_exception& e) {
        // This is also acceptable behavior if implementation rejects too small values
    }
    
    // Zero values
    double d_zero_pos = 0.0;
    UTEST_ASSERT_EQUALS(0.0f, numeric_cast<float>(d_zero_pos));
    UTEST_ASSERT_FALSE(std::signbit(numeric_cast<float>(d_zero_pos)));
    
    double d_zero_neg = -0.0;
    UTEST_ASSERT_EQUALS(0.0f, numeric_cast<float>(d_zero_neg));
    UTEST_ASSERT_TRUE(std::signbit(numeric_cast<float>(d_zero_neg)));
    
    // Precision loss tests
    // Values that lose precision but are in range
    double precise_value = 1.0 + std::pow(2.0, -25); // Just beyond float precision
    UTEST_ASSERT_EQUALS(1.0f, numeric_cast<float>(precise_value)); // Will round to 1.0f
    
    double precise_large = 16777216.0 + 1.0; // 2^24 + 1, just beyond float precision
    UTEST_ASSERT_EQUALS(16777216.0f, numeric_cast<float>(precise_large)); // Will round to 2^24
    
    // Subnormal values in double that convert to normal float values
    if (std::numeric_limits<double>::has_denorm == std::denorm_present) {
        double small_but_normal_in_float = std::numeric_limits<double>::min() * 2.0;
        if (small_but_normal_in_float >= std::numeric_limits<float>::min()) {
            UTEST_ASSERT_EQUALS(static_cast<float>(small_but_normal_in_float), 
                               numeric_cast<float>(small_but_normal_in_float));
        }
    }
}

// =============================================================================
// CHAR_CAST SPECIFIC TESTS  
// =============================================================================

// Test char_cast basic functionality
UTEST_FUNC_DEF(CharCastBasic) {
    // Basic char type conversions (always safe - reinterpret behavior)
    char c1 = 'A';
    unsigned char uc1 = 65;
    signed char sc1 = 'B';
    
    UTEST_ASSERT_EQUALS('A', char_cast<char>(uc1));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>('A'), char_cast<unsigned char>(c1));
    UTEST_ASSERT_EQUALS(static_cast<signed char>('A'), char_cast<signed char>(c1));
    UTEST_ASSERT_EQUALS(static_cast<char>('B'), char_cast<char>(sc1));
}

// Test char_cast with boundary values and edge cases
UTEST_FUNC_DEF(CharCastBoundary) {
    // Test boundary values for char types using numeric_limits
    unsigned char max_uchar = std::numeric_limits<unsigned char>::max();
    signed char max_schar = std::numeric_limits<signed char>::max();
    signed char min_schar = std::numeric_limits<signed char>::min();
    
    // char_cast always succeeds (reinterpret behavior)
    UTEST_ASSERT_EQUALS(static_cast<char>(max_schar), char_cast<char>(max_schar));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(max_schar), char_cast<unsigned char>(max_schar));
    
    // Test maximum unsigned char to other char types
    UTEST_ASSERT_EQUALS(static_cast<char>(max_uchar), char_cast<char>(max_uchar));
    UTEST_ASSERT_EQUALS(static_cast<signed char>(max_uchar), char_cast<signed char>(max_uchar));
    
    // Test minimum signed char to other char types
    UTEST_ASSERT_EQUALS(static_cast<char>(min_schar), char_cast<char>(min_schar));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(min_schar), char_cast<unsigned char>(min_schar));
    
    // Negative signed char to unsigned char (reinterpret behavior)
    signed char neg_one = -1;
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(-1), char_cast<unsigned char>(neg_one));
    UTEST_ASSERT_EQUALS(255, static_cast<int>(char_cast<unsigned char>(neg_one))); // -1 becomes 255
}

// Test comprehensive int-to-char conversions using numeric_limits
UTEST_FUNC_DEF(IntToCharConversions) {
    // ASCII printable range (32-126) - always safe with numeric_cast
    UTEST_ASSERT_EQUALS('A', numeric_cast<char>(65));
    UTEST_ASSERT_EQUALS('Z', numeric_cast<char>(90));
    UTEST_ASSERT_EQUALS('a', numeric_cast<char>(97));
    UTEST_ASSERT_EQUALS('z', numeric_cast<char>(122));
    UTEST_ASSERT_EQUALS('0', numeric_cast<char>(48));
    UTEST_ASSERT_EQUALS('9', numeric_cast<char>(57));
    UTEST_ASSERT_EQUALS(' ', numeric_cast<char>(32));
    
    // Test boundary cases using numeric_limits
    UTEST_ASSERT_EQUALS(std::numeric_limits<char>::max(), 
                        numeric_cast<char>(static_cast<int>(std::numeric_limits<char>::max())));
    UTEST_ASSERT_EQUALS(std::numeric_limits<char>::min(), 
                        numeric_cast<char>(static_cast<int>(std::numeric_limits<char>::min())));
    
    // Test unsigned char range using numeric_limits
    UTEST_ASSERT_EQUALS(std::numeric_limits<unsigned char>::max(), 
                        numeric_cast<unsigned char>(static_cast<int>(std::numeric_limits<unsigned char>::max())));
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(128), 
                        numeric_cast<unsigned char>(128));
    
    // Values outside char range should throw
    UTEST_ASSERT_THROWS([](){ numeric_cast<char>(256); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<char>(1000); });
    if (std::numeric_limits<char>::min() >= 0) { // unsigned char
        UTEST_ASSERT_THROWS([](){ numeric_cast<char>(-1); });
    } else { // signed char
        UTEST_ASSERT_THROWS([](){ numeric_cast<char>(-200); });
    }
    
    // Values outside unsigned char range should throw  
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned char>(256); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned char>(-1); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned char>(1000); });
}

// Test char_cast for different char type conversions (not int-to-char)
UTEST_FUNC_DEF(CharToCharWithCharCast) {
    // char_cast only works between char types - test this behavior
    
    // Test char to other char types
    char c_val = 'A';
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>('A'), char_cast<unsigned char>(c_val));
    UTEST_ASSERT_EQUALS(static_cast<signed char>('A'), char_cast<signed char>(c_val));
    
    // Test unsigned char to other char types
    unsigned char uc_val = 200; // Value that would be problematic for signed char
    char from_uc = char_cast<char>(uc_val);
    signed char from_uc_signed = char_cast<signed char>(uc_val);
    
    // These should match static_cast behavior (reinterpret, no validation)
    UTEST_ASSERT_EQUALS(static_cast<char>(uc_val), from_uc);
    UTEST_ASSERT_EQUALS(static_cast<signed char>(uc_val), from_uc_signed);
    
    // Test signed char to other char types
    signed char sc_neg = -50;
    char from_sc = char_cast<char>(sc_neg);
    unsigned char from_sc_unsigned = char_cast<unsigned char>(sc_neg);
    
    // Should match static_cast behavior
    UTEST_ASSERT_EQUALS(static_cast<char>(sc_neg), from_sc);
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(sc_neg), from_sc_unsigned);
    
    // Test boundary values using numeric_limits
    unsigned char max_uchar = std::numeric_limits<unsigned char>::max();
    signed char min_schar = std::numeric_limits<signed char>::min();
    
    // char_cast should work without throwing (unlike numeric_cast)
    char from_max_uc = char_cast<char>(max_uchar);
    unsigned char from_min_sc = char_cast<unsigned char>(min_schar);
    
    UTEST_ASSERT_EQUALS(static_cast<char>(max_uchar), from_max_uc);
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(min_schar), from_min_sc);
    
    // Compare with numeric_cast behavior for int-to-char
    // numeric_cast does validation, char_cast does not
    int safe_int = 65;
    UTEST_ASSERT_EQUALS('A', numeric_cast<char>(safe_int));
    
    int unsafe_int = 300;
    UTEST_ASSERT_THROWS([unsafe_int](){ numeric_cast<char>(unsafe_int); });
    // Note: char_cast cannot be used with int, it's char-to-char only
}

// =============================================================================
// MACRO TESTS
// =============================================================================

// Test macro versions of the functions
UTEST_FUNC_DEF(MacroVersions) {
    // Test NUMERIC_CAST macro
    int value = 42;
    UTEST_ASSERT_EQUALS(42u, NUMERIC_CAST(unsigned int, value));
    UTEST_ASSERT_EQUALS('*', NUMERIC_CAST(char, 42));
    
    // Test CHAR_CAST macro
    char c = 'X';
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>('X'), CHAR_CAST(unsigned char, c));
    
    signed char sc = -50;
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(-50), CHAR_CAST(unsigned char, sc));
}

// Test macro exception information and location reporting
UTEST_FUNC_DEF(MacroExceptionInfo) {
    // Test that NUMERIC_CAST macro provides file and line information in exceptions
    try {
        int negative = -1;
        NUMERIC_CAST(unsigned int, negative);
        UTEST_ASSERT_TRUE(false); // Should not reach here
    } catch (const ncast::cast_exception& e) {
        std::string msg = e.what();
        // Should contain file and line information
        UTEST_ASSERT_STR_CONTAINS(msg, "test_ncast.cpp");
        UTEST_ASSERT_STR_CONTAINS(msg, "Line");
    }
    
    // Test that regular function calls don't have as much location info
    try {
        numeric_cast<unsigned int>(-1);
        UTEST_ASSERT_TRUE(false); // Should not reach here  
    } catch (const ncast::cast_exception& e) {
        std::string msg = e.what();
        // Should contain basic error info but not necessarily file/line
        UTEST_ASSERT_STR_CONTAINS(msg, "cast");
    }
}

// =============================================================================
// INTEGRATION AND EDGE CASE TESTS
// =============================================================================

// Test combination scenarios and edge cases
UTEST_FUNC_DEF(IntegrationTests) {
    // Test chained conversions
    int original = 65;
    char as_char = numeric_cast<char>(original);
    int back_to_int = numeric_cast<int>(as_char);
    UTEST_ASSERT_EQUALS(original, back_to_int);
    
    // Test different paths to same result
    unsigned char uc1 = numeric_cast<unsigned char>(65);
    unsigned char uc2 = char_cast<unsigned char>('A');
    UTEST_ASSERT_EQUALS(uc1, uc2);
    
    // Test zero in different types
    UTEST_ASSERT_EQUALS(0, numeric_cast<int>(0.0));
    UTEST_ASSERT_EQUALS('\0', numeric_cast<char>(0));
    UTEST_ASSERT_EQUALS(0u, numeric_cast<unsigned int>(0));
    
    // Test one in different types
    UTEST_ASSERT_EQUALS(1, numeric_cast<int>(1.0));
    UTEST_ASSERT_EQUALS(1u, numeric_cast<unsigned int>(1));
    UTEST_ASSERT_EQUALS(static_cast<char>(1), numeric_cast<char>(1));
    
    // Test boundary interactions
    UTEST_ASSERT_EQUALS(std::numeric_limits<unsigned char>::max(), 
                        numeric_cast<unsigned char>(std::numeric_limits<unsigned char>::max()));
    UTEST_ASSERT_EQUALS(std::numeric_limits<signed char>::min(), 
                        numeric_cast<signed char>(std::numeric_limits<signed char>::min()));
}

// Test infinity conversions
// This function centralizes all infinity-related tests to ensure consistent handling
UTEST_FUNC_DEF(InfinityConversions) {
    // Float to double infinity conversions (should always work)
    float f_inf_pos = std::numeric_limits<float>::infinity();
    double d_inf_pos = numeric_cast<double>(f_inf_pos);
    UTEST_ASSERT_TRUE(std::isinf(d_inf_pos));
    UTEST_ASSERT_TRUE(d_inf_pos > 0);
    
    float f_inf_neg = -std::numeric_limits<float>::infinity();
    double d_inf_neg_result = numeric_cast<double>(f_inf_neg);
    UTEST_ASSERT_TRUE(std::isinf(d_inf_neg_result));
    UTEST_ASSERT_TRUE(d_inf_neg_result < 0);
    
    // Double to float infinity conversions (should work with proper implementation)
    double d_inf_pos_src = std::numeric_limits<double>::infinity();
    float f_inf_pos_result = numeric_cast<float>(d_inf_pos_src);
    UTEST_ASSERT_TRUE(std::isinf(f_inf_pos_result));
    UTEST_ASSERT_TRUE(f_inf_pos_result > 0);
    
    double d_inf_neg_src = -std::numeric_limits<double>::infinity();
    float f_inf_neg_result = numeric_cast<float>(d_inf_neg_src);
    UTEST_ASSERT_TRUE(std::isinf(f_inf_neg_result));
    UTEST_ASSERT_TRUE(f_inf_neg_result < 0);
    
    // Infinity to integral types (should throw)
    float f_inf = std::numeric_limits<float>::infinity();
    UTEST_ASSERT_THROWS([f_inf](){ numeric_cast<int>(f_inf); });
    UTEST_ASSERT_THROWS([f_inf](){ numeric_cast<unsigned int>(f_inf); });
    UTEST_ASSERT_THROWS([f_inf](){ numeric_cast<char>(f_inf); });
    
    double d_inf = std::numeric_limits<double>::infinity();
    UTEST_ASSERT_THROWS([d_inf](){ numeric_cast<long>(d_inf); });
    UTEST_ASSERT_THROWS([d_inf](){ numeric_cast<unsigned long>(d_inf); });
    UTEST_ASSERT_THROWS([d_inf](){ numeric_cast<short>(d_inf); });
    
    // Negative infinity to unsigned types (should throw for two reasons)
    float f_neg_inf_test = -std::numeric_limits<float>::infinity();
    UTEST_ASSERT_THROWS([f_neg_inf_test](){ numeric_cast<unsigned int>(f_neg_inf_test); });
    
    double d_neg_inf_test = -std::numeric_limits<double>::infinity();
    UTEST_ASSERT_THROWS([d_neg_inf_test](){ numeric_cast<unsigned long>(d_neg_inf_test); });
}

// Test NaN (Not-a-Number) conversions
// This function centralizes all NaN-related tests to ensure consistent handling
UTEST_FUNC_DEF(NaNConversions) {
    // Basic NaN conversions between floating point types (should work)
    float f_nan = std::numeric_limits<float>::quiet_NaN();
    double d_nan_from_float = numeric_cast<double>(f_nan);
    UTEST_ASSERT_TRUE(std::isnan(d_nan_from_float));
    
    double d_nan = std::numeric_limits<double>::quiet_NaN();
    float f_nan_from_double = numeric_cast<float>(d_nan);
    UTEST_ASSERT_TRUE(std::isnan(f_nan_from_double));
    
    // Signaling NaN conversions
    float f_snan = std::numeric_limits<float>::signaling_NaN();
    double d_snan_result = numeric_cast<double>(f_snan);
    UTEST_ASSERT_TRUE(std::isnan(d_snan_result));
    
    double d_snan = std::numeric_limits<double>::signaling_NaN();
    float f_snan_result = numeric_cast<float>(d_snan);
    UTEST_ASSERT_TRUE(std::isnan(f_snan_result));
    
    // NaN to integral types (should throw)
    UTEST_ASSERT_THROWS([f_nan](){ numeric_cast<int>(f_nan); });
    UTEST_ASSERT_THROWS([f_nan](){ numeric_cast<unsigned int>(f_nan); });
    UTEST_ASSERT_THROWS([f_nan](){ numeric_cast<char>(f_nan); });
    
    UTEST_ASSERT_THROWS([d_nan](){ numeric_cast<long>(d_nan); });
    UTEST_ASSERT_THROWS([d_nan](){ numeric_cast<unsigned long>(d_nan); });
    UTEST_ASSERT_THROWS([d_nan](){ numeric_cast<short>(d_nan); });
    
    // NaN sign handling (NaNs can be positive or negative)
    float f_neg_nan = -std::numeric_limits<float>::quiet_NaN();
    double d_neg_nan_result = numeric_cast<double>(f_neg_nan);
    UTEST_ASSERT_TRUE(std::isnan(d_neg_nan_result));
    
    double d_neg_nan = -std::numeric_limits<double>::quiet_NaN();
    float f_neg_nan_result = numeric_cast<float>(d_neg_nan);
    UTEST_ASSERT_TRUE(std::isnan(f_neg_nan_result));
    
    // Test for NaN payload preservation (if possible)
    // This is implementation-defined, so we only check that the result is NaN
    
    // Convert between different types of NaNs
    float f_qnan_src = std::numeric_limits<float>::quiet_NaN();
    double d_from_float_qnan = numeric_cast<double>(f_qnan_src);
    UTEST_ASSERT_TRUE(std::isnan(d_from_float_qnan));
    
    double d_snan_src = std::numeric_limits<double>::signaling_NaN();
    float f_from_double_snan = numeric_cast<float>(d_snan_src);
    UTEST_ASSERT_TRUE(std::isnan(f_from_double_snan));
    
    // Ensure NaN is still NaN after multiple conversions
    float f_nan_orig = std::numeric_limits<float>::quiet_NaN();
    double d_nan_mid = numeric_cast<double>(f_nan_orig);
    float f_nan_final = numeric_cast<float>(d_nan_mid);
    UTEST_ASSERT_TRUE(std::isnan(f_nan_final));
}

int main() {
    UTEST_PROLOG();
    
    // Basic functionality tests
    UTEST_FUNC(NumericCastBasic);
    UTEST_FUNC(NumericCastCharBasic);
    
    // Overflow and underflow tests
    UTEST_FUNC(SignedToUnsignedOverflow);
    UTEST_FUNC(UnsignedToSignedOverflow);
    UTEST_FUNC(NarrowingConversions);
    UTEST_FUNC(IntegerSizeEdgeCases);
    
    // Floating point tests
    UTEST_FUNC(FloatToIntConversion);
    UTEST_FUNC(IntToFloatConversion);
    UTEST_FUNC(FloatToDoubleConversion);
    UTEST_FUNC(DoubleToFloatConversion);
    UTEST_FUNC(InfinityConversions);
    UTEST_FUNC(NaNConversions);
    
    // char_cast specific tests
    UTEST_FUNC(CharCastBasic);
    UTEST_FUNC(CharCastBoundary);
    UTEST_FUNC(IntToCharConversions);
    UTEST_FUNC(CharToCharWithCharCast);
    
    // Macro tests
    UTEST_FUNC(MacroVersions);
    UTEST_FUNC(MacroExceptionInfo);
    
    // Integration tests
    UTEST_FUNC(IntegrationTests);
    
    UTEST_EPILOG();
    
    return 0;
}
