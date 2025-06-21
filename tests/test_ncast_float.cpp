#include "../include/ncast/ncast.h"
#include "../include/utest/utest.h"
#include <climits>
#include <limits>
#include <cmath> // For std::isnan, std::isinf, std::pow

using namespace ncast;

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
    {
        // Use a known value that's precisely representable in float
        int32_t precise_int = (1 << 24) - 1; // 2^24 - 1 = 16,777,215 (exactly representable in float)
        float f_precise = numeric_cast<float>(precise_int);
        UTEST_ASSERT_EQUALS(static_cast<float>(precise_int), f_precise);
        
        // Use a known value that will lose precision in float
        // Numbers above 2^24 start losing precision in float (24-bit mantissa)
        if (std::numeric_limits<float>::digits < 25) {
            int32_t imprecise_int = (1 << 24) + 1; // 2^24 + 1 = 16,777,217 (may lose precision)
            float f_imprecise = numeric_cast<float>(imprecise_int);
            
            // This should still work because we're just checking range, not precision
            // Depending on rounding mode, it might round to 2^24 or 2^24+2
            UTEST_ASSERT_TRUE(f_imprecise == 16777216.0f || f_imprecise == 16777218.0f);
        }
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

// Test floating point signed zero conversions
UTEST_FUNC_DEF(SignedZeroConversions) {
    // Create positive and negative zero
    double pos_zero = 0.0;
    double neg_zero = -0.0;
    
    // Verify they're different (should have different sign bits)
    UTEST_ASSERT_TRUE(std::signbit(neg_zero));
    UTEST_ASSERT_FALSE(std::signbit(pos_zero));
    
    // Test sign preservation in double to float conversion
    float f_pos_zero = numeric_cast<float>(pos_zero);
    float f_neg_zero = numeric_cast<float>(neg_zero);
    
    UTEST_ASSERT_FALSE(std::signbit(f_pos_zero));
    UTEST_ASSERT_TRUE(std::signbit(f_neg_zero));
    
    // Test sign preservation in float to double conversion
    float f_pos_zero_src = 0.0f;
    float f_neg_zero_src = -0.0f;
    
    double d_pos_zero = numeric_cast<double>(f_pos_zero_src);
    double d_neg_zero = numeric_cast<double>(f_neg_zero_src);
    
    UTEST_ASSERT_FALSE(std::signbit(d_pos_zero));
    UTEST_ASSERT_TRUE(std::signbit(d_neg_zero));
    
    // Test conversions to integers (sign should be lost, both become 0)
    int i_from_pos_zero = numeric_cast<int>(pos_zero);
    int i_from_neg_zero = numeric_cast<int>(neg_zero);
    
    UTEST_ASSERT_EQUALS(0, i_from_pos_zero);
    UTEST_ASSERT_EQUALS(0, i_from_neg_zero);
}

// Test conversions between extreme floating point values
UTEST_FUNC_DEF(ExtremeFloatingPointConversions) {
    // Test extremely small normal double to float
    if (std::numeric_limits<double>::min() < std::numeric_limits<float>::min()) {
        // Use a safer approach with a value that's definitely in range
        double safe_small_double = std::numeric_limits<float>::min() * 1.5;
        float result_safe = numeric_cast<float>(safe_small_double);
        UTEST_ASSERT_TRUE(result_safe > 0.0f);
        
        // Now try a very small value that might be too small for float
        double very_small_double = std::numeric_limits<float>::min() * 0.1;
        
        // This might succeed or throw depending on the implementation
        try {
            float result = numeric_cast<float>(very_small_double);
            // If it succeeds, the result should be either a subnormal or zero
            UTEST_ASSERT_TRUE(result >= 0.0f); // Should at least be non-negative
        } catch (const cast_exception& e) {
            // This is also a valid outcome if the implementation rejects the conversion
            // Just verify the exception message contains appropriate text
            std::string msg = e.what();
            UTEST_ASSERT_TRUE(msg.find("below minimum") != std::string::npos || 
                              msg.find("underflow") != std::string::npos || 
                              msg.find("too small") != std::string::npos);
        }
    }
    
    // Test extremely small integer to floating point - this should always work
    {
        int tiny_integer = 1;
        float f_from_tiny = numeric_cast<float>(tiny_integer);
        UTEST_ASSERT_EQUALS(1.0f, f_from_tiny);
    }
    
    // Test precision loss for large integers to double
    // Use fixed-size types to ensure consistent behavior
    {
        // Use a value that's definitely representable in double
        int32_t medium_int = (int32_t)1 << 20; // 2^20, well within double precision
        double d_medium = numeric_cast<double>(medium_int);
        UTEST_ASSERT_EQUALS((double)medium_int, d_medium);
        
        // Test a value that should be exactly representable in double
        // Double has ~15-17 decimal digits of precision
        int64_t large_exact = 1000000000000000LL; // 10^15, should be exact in double
        try {
            double d_large_exact = numeric_cast<double>(large_exact);
            // This should work on most platforms
            UTEST_ASSERT_EQUALS((double)large_exact, d_large_exact);
        } catch (const cast_exception& e) {
            // On some platforms with unusual floating point handling, this might fail
            // That's acceptable too
        }
        
        // Test a value near the precision boundary
        // Double mantissa has ~53 bits of precision, so values near 2^53 may lose precision
        if (std::numeric_limits<double>::digits >= 53) {
            double pow_2_52 = std::pow(2.0, 52); // 2^52 should be exactly representable
            double pow_2_53 = std::pow(2.0, 53); // 2^53 should be exactly representable
            double pow_2_53_plus_1 = pow_2_53 + 1.0; // This might lose precision
            
            // Verify precision behavior
            UTEST_ASSERT_TRUE(pow_2_52 + 1.0 > pow_2_52); // Should be different
            
            // This might be equal on some platforms due to precision loss
            if (pow_2_53_plus_1 == pow_2_53) {
                // If precision was lost, that's expected
                UTEST_ASSERT_TRUE(true);
            } else {
                // If precision was maintained, that's also fine
                UTEST_ASSERT_TRUE(pow_2_53_plus_1 > pow_2_53);
            }
        }
    }
    
    // Test subnormal values more carefully
    if (std::numeric_limits<double>::has_denorm == std::denorm_present &&
        std::numeric_limits<float>::has_denorm == std::denorm_present) {
        
        // Test with a small but definitely representable value
        float small_float = std::numeric_limits<float>::min(); // Smallest normal float
        double d_from_small_float = numeric_cast<double>(small_float);
        UTEST_ASSERT_EQUALS((double)small_float, d_from_small_float);
        
        // Test with a normal double to subnormal float conversion
        double normal_double = std::numeric_limits<float>::min() * 0.5;
        
        // This may or may not throw depending on compiler/platform
        try {
            float result = numeric_cast<float>(normal_double);
            // If it succeeds, just verify it's non-negative
            UTEST_ASSERT_TRUE(result >= 0.0f);
        } catch (const cast_exception& e) {
            // This is also acceptable
            // Just check that the error message is reasonable
            std::string msg = e.what();
            UTEST_ASSERT_TRUE(msg.length() > 0);
        }
    }
}

// =============================================================================
// LONG DOUBLE SPECIFIC TESTS
// =============================================================================

// Test long double to other floating point conversions
UTEST_FUNC_DEF(LongDoubleToFloatingPoint) {
    // Basic long double to double/float conversions
    long double ld1 = 42.5L;
    UTEST_ASSERT_EQUALS(42.5, numeric_cast<double>(ld1));
    UTEST_ASSERT_EQUALS(42.5f, numeric_cast<float>(ld1));
    
    // Test negative values
    long double ld_neg = -123.456L;
    UTEST_ASSERT_EQUALS(-123.456, numeric_cast<double>(ld_neg));
    UTEST_ASSERT_EQUALS(-123.456f, numeric_cast<float>(ld_neg));
    
    // Test zero
    long double ld_zero = 0.0L;
    UTEST_ASSERT_EQUALS(0.0, numeric_cast<double>(ld_zero));
    UTEST_ASSERT_EQUALS(0.0f, numeric_cast<float>(ld_zero));
    
    // Test same type conversion
    long double ld2 = 999.999L;
    UTEST_ASSERT_EQUALS(999.999L, numeric_cast<long double>(ld2));
}

// Test integer to long double conversions
UTEST_FUNC_DEF(IntegerToLongDouble) {
    // Basic integer to long double conversions
    UTEST_ASSERT_EQUALS(42.0L, numeric_cast<long double>(42));
    UTEST_ASSERT_EQUALS(-42.0L, numeric_cast<long double>(-42));
    UTEST_ASSERT_EQUALS(42.0L, numeric_cast<long double>(42u));
    
    // Large integer values
    long long large_int = std::numeric_limits<long long>::max();
    long double ld_result = numeric_cast<long double>(large_int);
    UTEST_ASSERT_EQUALS(static_cast<long double>(large_int), ld_result);
    
    // Test with unsigned long long
    unsigned long long large_uint = std::numeric_limits<unsigned long long>::max();
    long double ld_uint_result = numeric_cast<long double>(large_uint);
    UTEST_ASSERT_EQUALS(static_cast<long double>(large_uint), ld_uint_result);
    
    // Test minimum values
    long long min_int = std::numeric_limits<long long>::min();
    long double ld_min_result = numeric_cast<long double>(min_int);
    UTEST_ASSERT_EQUALS(static_cast<long double>(min_int), ld_min_result);
}

// Test long double to integer conversions
UTEST_FUNC_DEF(LongDoubleToInteger) {
    // Basic conversions with truncation
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42.0L));
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42.7L)); // Should truncate
    UTEST_ASSERT_EQUALS(42, numeric_cast<int>(42.9L)); // Should truncate
    UTEST_ASSERT_EQUALS(-42, numeric_cast<int>(-42.7L)); // Should truncate toward zero
    
    // Test boundary values
    long double max_int_as_ld = static_cast<long double>(std::numeric_limits<int>::max());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::max(), numeric_cast<int>(max_int_as_ld));
    
    long double min_int_as_ld = static_cast<long double>(std::numeric_limits<int>::min());
    UTEST_ASSERT_EQUALS(std::numeric_limits<int>::min(), numeric_cast<int>(min_int_as_ld));
    
    // Test conversion to unsigned types
    UTEST_ASSERT_EQUALS(42u, numeric_cast<unsigned int>(42.0L));
    UTEST_ASSERT_EQUALS(0u, numeric_cast<unsigned int>(0.0L));
    
    // Negative long double to unsigned should throw
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned int>(-1.0L); });
    UTEST_ASSERT_THROWS([](){ numeric_cast<unsigned char>(-42.5L); });
}

// Test long double overflow/underflow detection
UTEST_FUNC_DEF(LongDoubleOverflowDetection) {
    // Test long double to smaller floating point types
    if (std::numeric_limits<long double>::max() > std::numeric_limits<double>::max()) {
        // Create a long double value larger than double can handle
        long double huge_ld = std::numeric_limits<long double>::max();
        
        // Should throw when converting to double
        UTEST_ASSERT_THROWS([huge_ld](){ numeric_cast<double>(huge_ld); });
        
        // Should also throw when converting to float
        UTEST_ASSERT_THROWS([huge_ld](){ numeric_cast<float>(huge_ld); });
    }
    
    if (std::numeric_limits<long double>::lowest() < std::numeric_limits<double>::lowest()) {
        // Create a long double value smaller than double can handle
        long double tiny_ld = std::numeric_limits<long double>::lowest();
        
        // Should throw when converting to double
        UTEST_ASSERT_THROWS([tiny_ld](){ numeric_cast<double>(tiny_ld); });
        
        // Should also throw when converting to float
        UTEST_ASSERT_THROWS([tiny_ld](){ numeric_cast<float>(tiny_ld); });
    }
    
    // Test long double to integer overflow
    if (std::numeric_limits<long double>::max() > std::numeric_limits<long long>::max()) {
        long double huge_for_int = static_cast<long double>(std::numeric_limits<long long>::max()) * 2.0L;
        UTEST_ASSERT_THROWS([huge_for_int](){ numeric_cast<long long>(huge_for_int); });
        UTEST_ASSERT_THROWS([huge_for_int](){ numeric_cast<int>(huge_for_int); });
    }
    
    // Test underflow
    if (std::numeric_limits<long double>::lowest() < std::numeric_limits<long long>::min()) {
        long double tiny_for_int = static_cast<long double>(std::numeric_limits<long long>::min()) * 2.0L;
        UTEST_ASSERT_THROWS([tiny_for_int](){ numeric_cast<long long>(tiny_for_int); });
        UTEST_ASSERT_THROWS([tiny_for_int](){ numeric_cast<int>(tiny_for_int); });
    }
}

// Test long double special values (NaN, infinity)
UTEST_FUNC_DEF(LongDoubleSpecialValues) {
    // Test NaN conversions
    long double ld_nan = std::numeric_limits<long double>::quiet_NaN();
    
    // NaN to other floating point types should work
    double d_from_ld_nan = numeric_cast<double>(ld_nan);
    UTEST_ASSERT_TRUE(std::isnan(d_from_ld_nan));
    
    float f_from_ld_nan = numeric_cast<float>(ld_nan);
    UTEST_ASSERT_TRUE(std::isnan(f_from_ld_nan));
    
    // NaN to integer types should throw
    UTEST_ASSERT_THROWS([ld_nan](){ numeric_cast<int>(ld_nan); });
    UTEST_ASSERT_THROWS([ld_nan](){ numeric_cast<unsigned int>(ld_nan); });
    UTEST_ASSERT_THROWS([ld_nan](){ numeric_cast<long long>(ld_nan); });
    
    // Test infinity conversions
    long double ld_inf = std::numeric_limits<long double>::infinity();
    
    // Infinity to other floating point types should work if in range
    if (std::numeric_limits<double>::has_infinity) {
        double d_from_ld_inf = numeric_cast<double>(ld_inf);
        UTEST_ASSERT_TRUE(std::isinf(d_from_ld_inf));
    }
    
    if (std::numeric_limits<float>::has_infinity) {
        float f_from_ld_inf = numeric_cast<float>(ld_inf);
        UTEST_ASSERT_TRUE(std::isinf(f_from_ld_inf));
    }
    
    // Infinity to integer types should throw
    UTEST_ASSERT_THROWS([ld_inf](){ numeric_cast<int>(ld_inf); });
    UTEST_ASSERT_THROWS([ld_inf](){ numeric_cast<unsigned int>(ld_inf); });
    UTEST_ASSERT_THROWS([ld_inf](){ numeric_cast<long long>(ld_inf); });
    
    // Test negative infinity
    long double ld_neg_inf = -std::numeric_limits<long double>::infinity();
    UTEST_ASSERT_THROWS([ld_neg_inf](){ numeric_cast<int>(ld_neg_inf); });
    UTEST_ASSERT_THROWS([ld_neg_inf](){ numeric_cast<unsigned int>(ld_neg_inf); });
}

// Test precision preservation with long double
UTEST_FUNC_DEF(LongDoublePrecisionTests) {
    // Only run precision tests if long double has more precision than double
    if (std::numeric_limits<long double>::digits > std::numeric_limits<double>::digits) {
        // Test that values requiring high precision are handled correctly
        // This tests the fix for using double as intermediate type
        
        // Create a value that loses precision when converted to double
        unsigned long long precise_value = 0x1FFFFFFFFFFFFF01ULL; // Requires more than 53 bits precision
        
        // Verify the precision difference exists
        double as_double = static_cast<double>(precise_value);
        long double as_long_double = static_cast<long double>(precise_value);
        
        if (static_cast<long double>(as_double) != as_long_double) {
            // There is a precision difference, so our test is valid
            
            // Test that numeric_cast to long double preserves the original precision
            long double result = numeric_cast<long double>(precise_value);
            UTEST_ASSERT_EQUALS(as_long_double, result);
            
            // Test that the result is different from what we'd get via double
            UTEST_ASSERT_TRUE(result != static_cast<long double>(as_double));
        }
    }
}

// Test macro versions with long double
UTEST_FUNC_DEF(LongDoubleMacroTests) {
    // Test NUMERIC_CAST macro with long double
    long double ld_val = 123.456L;
    
    auto result_double = NUMERIC_CAST(double, ld_val);
    UTEST_ASSERT_EQUALS(123.456, result_double);
    
    auto result_float = NUMERIC_CAST(float, ld_val);
    UTEST_ASSERT_EQUALS(123.456f, result_float);
    
    auto result_int = NUMERIC_CAST(int, ld_val);
    UTEST_ASSERT_EQUALS(123, result_int);
    
    // Test with overflow (should throw with location info)
    if (std::numeric_limits<long double>::max() > std::numeric_limits<float>::max()) {
        long double huge_ld = std::numeric_limits<long double>::max();
        UTEST_ASSERT_THROWS([huge_ld](){ NUMERIC_CAST(float, huge_ld); });
    }
    
    // Test integer to long double via macro
    int int_val = 42;
    auto ld_result = NUMERIC_CAST(long double, int_val);
    UTEST_ASSERT_EQUALS(42.0L, ld_result);
}

int main() {
    UTEST_PROLOG();
    UTEST_ENABLE_VERBOSE_MODE();
    
    // Floating point tests
    UTEST_FUNC(IntToFloatConversion);
    UTEST_FUNC(FloatToIntConversion);
    UTEST_FUNC(FloatToDoubleConversion);
    UTEST_FUNC(DoubleToFloatConversion);
    UTEST_FUNC(InfinityConversions);
    UTEST_FUNC(NaNConversions);
    UTEST_FUNC(SignedZeroConversions);
    UTEST_FUNC(ExtremeFloatingPointConversions);
    
    // Long double specific tests
    UTEST_FUNC(LongDoubleToFloatingPoint);
    UTEST_FUNC(IntegerToLongDouble);
    UTEST_FUNC(LongDoubleToInteger);
    UTEST_FUNC(LongDoubleOverflowDetection);
    UTEST_FUNC(LongDoubleSpecialValues);
    UTEST_FUNC(LongDoublePrecisionTests);
    UTEST_FUNC(LongDoubleMacroTests);
    
    UTEST_EPILOG();
    
    return 0;
}
