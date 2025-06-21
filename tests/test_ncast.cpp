#include "../include/ncast/ncast.h"
#include "../include/utest/utest.h"
#include <climits>
#include <limits>

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
