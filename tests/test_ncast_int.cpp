#include "../include/ncast/ncast.h"
#include "../include/utest/utest.h"
#include <climits>
#include <limits>

using namespace ncast;

// =============================================================================
// INTEGER OVERFLOW AND UNDERFLOW TESTS
// =============================================================================

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

int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // Overflow and underflow tests
    UTEST_FUNC(SignedToUnsignedOverflow);
    UTEST_FUNC(UnsignedToSignedOverflow);
    UTEST_FUNC(NarrowingConversions);
    UTEST_FUNC(IntegerSizeEdgeCases);
    
    UTEST_EPILOG();
    
    return 0;
}
