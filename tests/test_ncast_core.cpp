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

// =============================================================================
// MACRO TESTS
// =============================================================================

// Test macro versions provide the same functionality as function versions
UTEST_FUNC_DEF(MacroVersions) {
    // Test NUMERIC_CAST macro
    int int_val = 42;
    unsigned int uint_val = 42u;
    
    UTEST_ASSERT_EQUALS(numeric_cast<unsigned int>(int_val), NUMERIC_CAST(unsigned int, int_val));
    UTEST_ASSERT_EQUALS(numeric_cast<int>(uint_val), NUMERIC_CAST(int, uint_val));
    
    // Test CHAR_CAST macro
    char c_val = 'A';
    UTEST_ASSERT_EQUALS(char_cast<unsigned char>(c_val), CHAR_CAST(unsigned char, c_val));
}

// Test that macro versions provide accurate exception information
UTEST_FUNC_DEF(MacroExceptionInfo) {
    try {
        auto result = NUMERIC_CAST(unsigned int, -1);
        (void)result; // Suppress unused variable warning
        UTEST_ASSERT_TRUE(false); // Should not reach here
    } catch (const cast_exception& e) {
        // Verify exception contains file information
        std::string what_msg = e.what();
        UTEST_ASSERT_TRUE(what_msg.find("test_ncast_core.cpp") != std::string::npos);
        UTEST_ASSERT_TRUE(what_msg.find("MacroExceptionInfo") != std::string::npos);
        
        // Check that the cast_exception accessors work
        UTEST_ASSERT_TRUE(!e.getFile().empty());
        UTEST_ASSERT_TRUE(e.getLine() > 0);
        UTEST_ASSERT_TRUE(!e.getFunction().empty());
    }
    
    // Test a successful cast doesn't throw
    auto valid_result = NUMERIC_CAST(int, 42u);
    UTEST_ASSERT_EQUALS(42, valid_result);
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

// Complex integration test combining multiple conversions
UTEST_FUNC_DEF(IntegrationTests) {
    // Chain multiple conversions
    int start_val = 100;
    float float_val = numeric_cast<float>(start_val);
    double double_val = numeric_cast<double>(float_val);
    int end_val = numeric_cast<int>(double_val);
    
    UTEST_ASSERT_EQUALS(start_val, end_val);
    
    // Test mixed signed/unsigned operations
    unsigned short us_val = 1000;
    int i_val = numeric_cast<int>(us_val);
    unsigned int ui_val = numeric_cast<unsigned int>(i_val);
    short s_val = numeric_cast<short>(ui_val);
    
    UTEST_ASSERT_EQUALS(1000, static_cast<int>(s_val));
    
    // Test char operations in chain
    char c1 = 'M';
    int ascii_val = numeric_cast<int>(c1);
    char c2 = numeric_cast<char>(ascii_val);
    unsigned char uc = char_cast<unsigned char>(c2);
    
    UTEST_ASSERT_EQUALS('M', static_cast<char>(uc));
}

int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // Basic functionality tests
    UTEST_FUNC(NumericCastBasic);
    UTEST_FUNC(NumericCastCharBasic);
    
    // Macro tests
    UTEST_FUNC(MacroVersions);
    UTEST_FUNC(MacroExceptionInfo);
    
    // Integration tests
    UTEST_FUNC(IntegrationTests);
    
    UTEST_EPILOG();
    
    return 0;
}
