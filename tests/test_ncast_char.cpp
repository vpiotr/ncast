#include "../include/ncast/ncast.h"
#include "../include/utest/utest.h"
#include <climits>
#include <limits>

using namespace ncast;

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

// Test comprehensive char type boundary interactions
UTEST_FUNC_DEF(CharTypeBoundaryTests) {
    // Test all three char types at their boundaries
    
    // Test char minimum and maximum values
    char char_min = std::numeric_limits<char>::min();
    char char_max = std::numeric_limits<char>::max();
    
    // Convert char boundaries to int and back
    int char_min_as_int = numeric_cast<int>(char_min);
    int char_max_as_int = numeric_cast<int>(char_max);
    
    UTEST_ASSERT_EQUALS(char_min, numeric_cast<char>(char_min_as_int));
    UTEST_ASSERT_EQUALS(char_max, numeric_cast<char>(char_max_as_int));
    
    // Test signed char boundaries
    signed char schar_min = std::numeric_limits<signed char>::min();
    signed char schar_max = std::numeric_limits<signed char>::max();
    
    int schar_min_as_int = numeric_cast<int>(schar_min);
    int schar_max_as_int = numeric_cast<int>(schar_max);
    
    UTEST_ASSERT_EQUALS(schar_min, numeric_cast<signed char>(schar_min_as_int));
    UTEST_ASSERT_EQUALS(schar_max, numeric_cast<signed char>(schar_max_as_int));
    
    // Test unsigned char boundaries
    unsigned char uchar_min = std::numeric_limits<unsigned char>::min(); // Always 0
    unsigned char uchar_max = std::numeric_limits<unsigned char>::max();
    
    int uchar_min_as_int = numeric_cast<int>(uchar_min);
    int uchar_max_as_int = numeric_cast<int>(uchar_max);
    
    UTEST_ASSERT_EQUALS(uchar_min, numeric_cast<unsigned char>(uchar_min_as_int));
    UTEST_ASSERT_EQUALS(uchar_max, numeric_cast<unsigned char>(uchar_max_as_int));
    
    // Test cross-type conversions at boundaries using char_cast
    // These should always work with char_cast
    
    // Convert between all char types at their respective limits
    char char_from_schar_max = char_cast<char>(schar_max);
    char char_from_schar_min = char_cast<char>(schar_min);
    char char_from_uchar_max = char_cast<char>(uchar_max);
    
    unsigned char uchar_from_schar_max = char_cast<unsigned char>(schar_max);
    unsigned char uchar_from_schar_min = char_cast<unsigned char>(schar_min);
    unsigned char uchar_from_char_max = char_cast<unsigned char>(char_max);
    unsigned char uchar_from_char_min = char_cast<unsigned char>(char_min);
    
    signed char schar_from_uchar_max = char_cast<signed char>(uchar_max);
    signed char schar_from_char_max = char_cast<signed char>(char_max);
    signed char schar_from_char_min = char_cast<signed char>(char_min);
    
    // All these should equal their static_cast equivalents
    UTEST_ASSERT_EQUALS(static_cast<char>(schar_max), char_from_schar_max);
    UTEST_ASSERT_EQUALS(static_cast<char>(schar_min), char_from_schar_min);
    UTEST_ASSERT_EQUALS(static_cast<char>(uchar_max), char_from_uchar_max);
    
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(schar_max), uchar_from_schar_max);
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(schar_min), uchar_from_schar_min);
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(char_max), uchar_from_char_max);
    UTEST_ASSERT_EQUALS(static_cast<unsigned char>(char_min), uchar_from_char_min);
    
    UTEST_ASSERT_EQUALS(static_cast<signed char>(uchar_max), schar_from_uchar_max);
    UTEST_ASSERT_EQUALS(static_cast<signed char>(char_max), schar_from_char_max);
    UTEST_ASSERT_EQUALS(static_cast<signed char>(char_min), schar_from_char_min);
}

// Test char operations with specific ASCII values
UTEST_FUNC_DEF(CharAsciiTests) {
    // Test common ASCII characters
    struct AsciiTest {
        char character;
        int ascii_value;
    };
    
    AsciiTest ascii_tests[] = {
        {'A', 65}, {'Z', 90}, {'a', 97}, {'z', 122},
        {'0', 48}, {'9', 57}, {' ', 32}, {'!', 33},
        {'~', 126}, {'\0', 0}, {'\t', 9}, {'\n', 10}
    };
    
    for (const auto& test : ascii_tests) {
        // Test int to char conversion
        UTEST_ASSERT_EQUALS(test.character, numeric_cast<char>(test.ascii_value));
        
        // Test char to int conversion
        UTEST_ASSERT_EQUALS(test.ascii_value, numeric_cast<int>(test.character));
        
        // Test char_cast between char types
        unsigned char as_uchar = char_cast<unsigned char>(test.character);
        signed char as_schar = char_cast<signed char>(test.character);
        
        // Should be equivalent to static_cast
        UTEST_ASSERT_EQUALS(static_cast<unsigned char>(test.character), as_uchar);
        UTEST_ASSERT_EQUALS(static_cast<signed char>(test.character), as_schar);
        
        // Convert back and verify
        char back_from_uchar = char_cast<char>(as_uchar);
        char back_from_schar = char_cast<char>(as_schar);
        
        UTEST_ASSERT_EQUALS(test.character, back_from_uchar);
        UTEST_ASSERT_EQUALS(test.character, back_from_schar);
    }
}

// Test char operations with extended ASCII (128-255)
UTEST_FUNC_DEF(ExtendedAsciiTests) {
    // Test extended ASCII range (only valid for unsigned char)
    for (int i = 128; i <= 255; ++i) {
        // Should work with unsigned char
        unsigned char uc = numeric_cast<unsigned char>(i);
        UTEST_ASSERT_EQUALS(static_cast<unsigned char>(i), uc);
        
        // Convert back to int
        int back_to_int = numeric_cast<int>(uc);
        UTEST_ASSERT_EQUALS(i, back_to_int);
        
        // Test char_cast to other char types
        char as_char = char_cast<char>(uc);
        signed char as_schar = char_cast<signed char>(uc);
        
        // Should match static_cast behavior
        UTEST_ASSERT_EQUALS(static_cast<char>(uc), as_char);
        UTEST_ASSERT_EQUALS(static_cast<signed char>(uc), as_schar);
        
        // These values may be negative when cast to signed char
        // but char_cast should handle this without throwing
        
        // Convert back using char_cast
        unsigned char back_from_char = char_cast<unsigned char>(as_char);
        unsigned char back_from_schar = char_cast<unsigned char>(as_schar);
        
        UTEST_ASSERT_EQUALS(uc, back_from_char);
        UTEST_ASSERT_EQUALS(uc, back_from_schar);
    }
    
    // Test that values > 255 throw with numeric_cast to any char type
    UTEST_ASSERT_THROWS([](){ numeric_cast<char>(256); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned char>(256); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<signed char>(256); });
    
    UTEST_ASSERT_THROWS([](){ numeric_cast<char>(1000); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned char>(1000); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<signed char>(1000); });
}

// Test char operations with negative values
UTEST_FUNC_DEF(NegativeCharTests) {
    // Test negative values with signed char
    for (int i = -128; i < 0; ++i) {
        // Should work with signed char if it can represent negative values
        if (std::numeric_limits<signed char>::min() <= i) {
            signed char sc = numeric_cast<signed char>(i);
            UTEST_ASSERT_EQUALS(static_cast<signed char>(i), sc);
            
            // Convert back to int
            int back_to_int = numeric_cast<int>(sc);
            UTEST_ASSERT_EQUALS(i, back_to_int);
            
            // Test char_cast to other char types
            char as_char = char_cast<char>(sc);
            unsigned char as_uchar = char_cast<unsigned char>(sc);
            
            // Should match static_cast behavior
            UTEST_ASSERT_EQUALS(static_cast<char>(sc), as_char);
            UTEST_ASSERT_EQUALS(static_cast<unsigned char>(sc), as_uchar);
            
            // Convert back using char_cast
            signed char back_from_char = char_cast<signed char>(as_char);
            signed char back_from_uchar = char_cast<signed char>(as_uchar);
            
            UTEST_ASSERT_EQUALS(sc, back_from_char);
            UTEST_ASSERT_EQUALS(sc, back_from_uchar);
        }
        
        // Negative values should always throw when casting to unsigned char
        UTEST_ASSERT_THROWS([i](){ numeric_cast<unsigned char>(i); });
        
        // Test with char type (depends on whether char is signed or unsigned)
        if (std::numeric_limits<char>::min() >= 0) {
            // char is unsigned, should throw
            UTEST_ASSERT_THROWS([i](){ numeric_cast<char>(i); });
        } else {
            // char is signed, should work if value is in range
            if (std::numeric_limits<char>::min() <= i) {
                char c = numeric_cast<char>(i);
                UTEST_ASSERT_EQUALS(static_cast<char>(i), c);
            } else {
                UTEST_ASSERT_THROWS([i](){ numeric_cast<char>(i); });
            }
        }
    }
}

int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // char_cast specific tests
    UTEST_FUNC(CharCastBasic);
    UTEST_FUNC(CharCastBoundary);
    UTEST_FUNC(IntToCharConversions);
    UTEST_FUNC(CharToCharWithCharCast);
    
    // Additional char-specific tests
    UTEST_FUNC(CharTypeBoundaryTests);
    UTEST_FUNC(CharAsciiTests);
    UTEST_FUNC(ExtendedAsciiTests);
    UTEST_FUNC(NegativeCharTests);
    
    UTEST_EPILOG();
    
    return 0;
}
