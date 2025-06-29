#ifndef NCAST_H
#define NCAST_H

/**
 * @file ncast.h
 * @brief Safe numeric casting header-only micro-library
 * @author Piotr Likus
 * @date June 21, 2025
 * 
 * This library provides safe casting functions that perform runtime validation
 * to prevent dangerous casts that could lead to unexpected behavior.
 * 
 * Features:
 * - numeric_cast: Safe casting between all numeric types and char
 * - char_cast: Safe casting between signed/unsigned chars only
 * - Runtime validation with comprehensive error reporting
 * - Compile-time validation for constant expressions (C++14+, optional)
 * - Macro versions with accurate location information
 * - Optional validation (can be disabled with NCAST_DISABLE_RUNTIME_VALIDATION)
 * - High-precision validation using long double intermediate calculations
 * - Enhanced support for long double with proper range checking
 * - C++11 compatible base functionality, enhanced features for newer standards
 * 
 * @code
 * #include <ncast/ncast.h>
 * 
 * // Safe numeric casting - works in all C++ standards
 * int value = 42;
 * unsigned int result = numeric_cast<unsigned int>(value); // Runtime validation
 * 
 * // C++14+ compile-time validation for constants (when supported)
 * constexpr int compile_time_value = 42;
 * constexpr unsigned int result2 = numeric_cast<unsigned int>(compile_time_value); // C++14+ only
 * 
 * // Safe char casting
 * signed char sc = 'A';
 * unsigned char uc = char_cast<unsigned char>(sc);
 * 
 * // Macro versions with location info
 * auto result2 = NUMERIC_CAST(unsigned int, value);
 * auto result3 = CHAR_CAST(unsigned char, sc);
 * @endcode
 */

#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>
#include <limits>
#include <cmath> // For std::isnan and std::isinf

// C++ standard detection and feature flags
#ifndef __cplusplus
#error "ncast requires a C++ compiler"
#endif

// Detect C++ standard version
#if __cplusplus >= 201402L
#define NCAST_HAS_CPP14 1
#else
#define NCAST_HAS_CPP14 0
#endif

#if __cplusplus >= 201703L
#define NCAST_HAS_CPP17 1
#else
#define NCAST_HAS_CPP17 0
#endif

#if __cplusplus >= 202002L
#define NCAST_HAS_CPP20 1
#else
#define NCAST_HAS_CPP20 0
#endif

// Feature detection
#if NCAST_HAS_CPP14 && !defined(NCAST_DISABLE_COMPILE_TIME_VALIDATION)
#define NCAST_CONSTEXPR_14 constexpr
#define NCAST_HAS_CONSTEXPR_VALIDATION 1
#else
#define NCAST_CONSTEXPR_14 
#define NCAST_HAS_CONSTEXPR_VALIDATION 0
#endif

#if NCAST_HAS_CPP20 && defined(__cpp_lib_is_constant_evaluated)
#define NCAST_HAS_IS_CONSTANT_EVALUATED 1
#else
#define NCAST_HAS_IS_CONSTANT_EVALUATED 0
#endif

// Cross-platform function name macro compatibility
#ifndef __PRETTY_FUNCTION__
    #ifdef _MSC_VER
        #define __PRETTY_FUNCTION__ __FUNCSIG__
    #elif defined(__GNUC__)
        #define __PRETTY_FUNCTION__ __PRETTY_FUNCTION__
    #else
        #define __PRETTY_FUNCTION__ __FUNCTION__
    #endif
#endif

namespace ncast {

/**
 * @brief Exception thrown when an unsafe cast is attempted
 */
class cast_exception : public std::runtime_error {
private:
    std::string message_;
    std::string file_;
    int line_;
    std::string function_;
    std::string formatted_message_;

    std::string format_message() const {
        std::ostringstream ss;
        ss << "Cast error: " << message_;
        if (!file_.empty() && line_ > 0) {
            ss << " (File: " << file_ 
               << ", Line: " << line_;
            if (!function_.empty()) {
                ss << ", Function: " << function_;
            }
            ss << ")";
        }
        return ss.str();
    }

public:
    /**
     * @brief Construct with basic error message
     */
    explicit cast_exception(const std::string& message)
        : std::runtime_error(message), 
          message_(message),
          line_(0) {
        formatted_message_ = format_message();
    }

    /**
     * @brief Construct with full location information
     */
    cast_exception(const std::string& message, const std::string& file, 
                   int line, const std::string& function)
        : std::runtime_error(message), 
          message_(message),
          file_(file), 
          line_(line), 
          function_(function) {
        formatted_message_ = format_message();
    }
    
    virtual ~cast_exception() = default;
    
    const std::string& getFile() const { return file_; }
    int getLine() const { return line_; }
    const std::string& getFunction() const { return function_; }
    
    virtual const char* what() const noexcept override {
        return formatted_message_.c_str();
    }
};

// Validation control macros
#ifndef NCAST_DISABLE_RUNTIME_VALIDATION
#define NCAST_ENABLE_RUNTIME_VALIDATION 1
#else
#define NCAST_ENABLE_RUNTIME_VALIDATION 0
#endif

namespace detail {
    /**
     * @brief Widening types used for safe range comparisons
     * 
     * These types provide maximum precision for intermediate calculations
     * during cast validation to ensure accurate range checking.
     */
    using widening_float_type = long double;    ///< Type for floating-point widening comparisons
    using widening_int_type = long long;        ///< Type for integer widening comparisons

#if NCAST_HAS_CONSTEXPR_VALIDATION
    /**
     * @brief Compile-time range validation utilities (C++14+ only)
     */
    namespace constexpr_validation {
        /**
         * @brief Check if a value is within target type range at compile time
         */
        template<typename ToType, typename FromType>
        NCAST_CONSTEXPR_14 bool is_in_range(FromType value) {
            return std::is_floating_point<FromType>::value
                ? (std::is_floating_point<ToType>::value
                    ? (value <= static_cast<FromType>(std::numeric_limits<ToType>::max()) &&
                       value >= static_cast<FromType>(std::numeric_limits<ToType>::lowest()))
                    : (value <= static_cast<FromType>(std::numeric_limits<ToType>::max()) &&
                       value >= static_cast<FromType>(std::numeric_limits<ToType>::lowest()) &&
                       value == static_cast<FromType>(static_cast<ToType>(value))))
                : (std::is_signed<FromType>::value && std::is_unsigned<ToType>::value && value < 0)
                    ? false
                    : (static_cast<widening_float_type>(value) <= static_cast<widening_float_type>(std::numeric_limits<ToType>::max()) &&
                       static_cast<widening_float_type>(value) >= static_cast<widening_float_type>(std::numeric_limits<ToType>::lowest()));
        }

        /**
         * @brief Constexpr implementation of numeric cast with compile-time validation
         */
        template<typename ToType, typename FromType>
        NCAST_CONSTEXPR_14 ToType numeric_cast_constexpr(FromType value) {
            static_assert(std::is_arithmetic<ToType>::value, "ToType must be a numeric type or char");
            static_assert(std::is_arithmetic<FromType>::value, "FromType must be a numeric type or char");
            
            return is_in_range<ToType>(value) 
                ? static_cast<ToType>(value)
                : throw cast_exception("Compile-time cast validation failed: value is out of range for target type");
        }
    }
#endif // NCAST_HAS_CONSTEXPR_VALIDATION

    // Forward declaration for numeric_cast_impl
    template<typename ToType, typename FromType>
    ToType numeric_cast_impl(FromType value, const char* file, int line, const char* function);

    /**
     * @brief Enhanced implementation with optional compile-time validation
     */

#if NCAST_HAS_CONSTEXPR_VALIDATION
    // C++14+ version with optional compile-time validation
    template<typename ToType, typename FromType>
    NCAST_CONSTEXPR_14 ToType numeric_cast_enhanced(FromType value, const char* file = "unknown", int line = 0, const char* function = "unknown") {
        // This will be evaluated at compile time for constant expressions in C++14+
        // and at runtime otherwise. The compiler automatically chooses the right path.
        return constexpr_validation::is_in_range<ToType>(value) 
            ? static_cast<ToType>(value)
            : (NCAST_ENABLE_RUNTIME_VALIDATION 
                ? throw cast_exception("Cast validation failed: value is out of range for target type", file, line, function)
                : static_cast<ToType>(value));
    }
#else
    // C++11 fallback - runtime validation only
    template<typename ToType, typename FromType>
    ToType numeric_cast_enhanced(FromType value, const char* file = "unknown", int line = 0, const char* function = "unknown") {
        return numeric_cast_impl<ToType>(value, file, line, function);
    }
#endif // NCAST_HAS_CONSTEXPR_VALIDATION

    /**
     * @brief Type trait to check if a type is a character type
     */
    template<typename T>
    struct is_char_type : std::false_type {};
    
    template<>
    struct is_char_type<char> : std::true_type {};
    
    template<>
    struct is_char_type<signed char> : std::true_type {};
    
    template<>
    struct is_char_type<unsigned char> : std::true_type {};

    /**
     * @brief Type trait to check if a type is numeric or char
     */
    template<typename T>
    struct is_numeric_or_char {
        static const bool value = std::is_arithmetic<T>::value;
    };

    // Base implementation declaration
    template<typename ToType, typename FromType, 
             bool IsFromFloatingPoint = std::is_floating_point<FromType>::value,
             bool IsToFloatingPoint = std::is_floating_point<ToType>::value>
    struct numeric_cast_validator;

    /**
     * @brief Helper to check NaN and infinity in floating point source types
     */
    template<typename FromType>
    bool check_floating_point_special(FromType value, const char* file, int line, const char* function) {
        // Allow NaN to be converted between floating point types
        if (std::isnan(value)) {
            std::ostringstream ss;
            ss << "Cannot convert NaN to non-floating-point type";
            throw cast_exception(ss.str(), file, line, function);
        }
        
        // Handle infinity to non-floating point types
        if (std::isinf(value)) {
            std::ostringstream ss;
            ss << "Cannot convert infinity to non-floating-point type";
            throw cast_exception(ss.str(), file, line, function);
        }
        
        return true;
    }

    // Specialization for floating-point source and floating-point target
    template<typename ToType, typename FromType>
    struct numeric_cast_validator<ToType, FromType, true, true> {
        static ToType validate(FromType value, const char* file, int line, const char* function) {
            // Allow NaN and infinity to be converted between floating point types
            if (std::isnan(value) || std::isinf(value)) {
                return static_cast<ToType>(value);
            }
            
            // Check for overflow/underflow
            if (value > static_cast<FromType>(std::numeric_limits<ToType>::max())) {
                std::ostringstream ss;
                ss << "Value (" << value << ") exceeds maximum for target type ("
                   << std::numeric_limits<ToType>::max() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            if (value < static_cast<FromType>(std::numeric_limits<ToType>::lowest())) {
                std::ostringstream ss;
                ss << "Value (" << value << ") is below minimum for target type ("
                   << std::numeric_limits<ToType>::lowest() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            return static_cast<ToType>(value);
        }
    };

    // Specialization for floating-point source and integral target
    template<typename ToType, typename FromType>
    struct numeric_cast_validator<ToType, FromType, true, false> {
        static ToType validate(FromType value, const char* file, int line, const char* function) {
            // Check for special values
            if (std::isnan(value)) {
                std::ostringstream ss;
                ss << "Cannot convert NaN to non-floating-point type";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            if (std::isinf(value)) {
                std::ostringstream ss;
                ss << "Cannot convert infinity to non-floating-point type";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            // Check for overflow/underflow
            if (value > static_cast<FromType>(std::numeric_limits<ToType>::max())) {
                std::ostringstream ss;
                ss << "Value (" << value << ") exceeds maximum for target type ("
                   << std::numeric_limits<ToType>::max() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            if (value < static_cast<FromType>(std::numeric_limits<ToType>::lowest())) {
                std::ostringstream ss;
                ss << "Value (" << value << ") is below minimum for target type ("
                   << std::numeric_limits<ToType>::lowest() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            return static_cast<ToType>(value);
        }
    };

    // Specialization for integral source and floating-point target
    template<typename ToType, typename FromType>
    struct numeric_cast_validator<ToType, FromType, false, true> {
        static ToType validate(FromType value, const char* file, int line, const char* function) {
            // Use widening_float_type for intermediate calculations to ensure maximum precision
            // and accuracy when the target type is long double or when high precision is needed
            widening_float_type wideningValue = static_cast<widening_float_type>(value);
            
            // Check for overflow/underflow using widening_float_type for maximum precision
            if (wideningValue > static_cast<widening_float_type>(std::numeric_limits<ToType>::max())) {
                std::ostringstream ss;
                ss << "Value (" << value << ") exceeds maximum for target type ("
                   << std::numeric_limits<ToType>::max() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            if (wideningValue < static_cast<widening_float_type>(std::numeric_limits<ToType>::lowest())) {
                std::ostringstream ss;
                ss << "Value (" << value << ") is below minimum for target type ("
                   << std::numeric_limits<ToType>::lowest() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            return static_cast<ToType>(value);
        }
    };

    // Specialization for integral source and integral target
    template<typename ToType, typename FromType>
    struct numeric_cast_validator<ToType, FromType, false, false> {
        static ToType validate(FromType value, const char* file, int line, const char* function) {
            // Check for signed to unsigned conversion with negative value
            if (std::is_signed<FromType>::value && std::is_unsigned<ToType>::value) {
                if (value < 0) {
                    std::ostringstream ss;
                    ss << "Attempt to cast negative value (" << value 
                       << ") to unsigned type";
                    throw cast_exception(ss.str(), file, line, function);
                }
            }
            
            // For integral types, use widening_float_type for range checks to ensure maximum precision
            // This handles cases where both FromType and ToType might be larger than long long
            widening_float_type wideningValue = static_cast<widening_float_type>(value);
            widening_float_type maxTarget = static_cast<widening_float_type>(std::numeric_limits<ToType>::max());
            widening_float_type minTarget = static_cast<widening_float_type>(std::numeric_limits<ToType>::lowest());
            
            if (wideningValue > maxTarget) {
                std::ostringstream ss;
                ss << "Value (" << value << ") exceeds maximum for target type ("
                   << std::numeric_limits<ToType>::max() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            if (wideningValue < minTarget) {
                std::ostringstream ss;
                ss << "Value (" << value << ") is below minimum for target type ("
                   << std::numeric_limits<ToType>::lowest() << ")";
                throw cast_exception(ss.str(), file, line, function);
            }
            
            return static_cast<ToType>(value);
        }
    };

    /**
     * @brief Helper function to perform safe numeric casting with validation
     */
    template<typename ToType, typename FromType>
    ToType numeric_cast_impl(FromType value, const char* file, int line, const char* function) {
        static_assert(is_numeric_or_char<ToType>::value, "ToType must be a numeric type or char");
        static_assert(is_numeric_or_char<FromType>::value, "FromType must be a numeric type or char");
        
#if !NCAST_ENABLE_RUNTIME_VALIDATION
        // Suppress unused parameter warnings when validation is disabled
        (void)file;
        (void)line;
        (void)function;
        return static_cast<ToType>(value);
#else
        return numeric_cast_validator<ToType, FromType>::validate(value, file, line, function);
#endif
    }

    /**
     * @brief Helper function to perform safe char casting with validation
     */
    template<typename ToType, typename FromType>
#if NCAST_HAS_CONSTEXPR_VALIDATION
    NCAST_CONSTEXPR_14 ToType char_cast_impl(FromType value, const char* /* file */, int /* line */, const char* /* function */) {
#else
    ToType char_cast_impl(FromType value, const char* /* file */, int /* line */, const char* /* function */) {
#endif
        static_assert(is_char_type<ToType>::value, "ToType must be a char type (char, signed char, unsigned char)");
        static_assert(is_char_type<FromType>::value, "FromType must be a char type (char, signed char, unsigned char)");
        
        // char_cast is always safe between char types, no validation needed
        return static_cast<ToType>(value);
    }
}

/**
 * @brief Safe cast between numeric types and char with validation
 * 
 * This function template provides safe casting between all numeric types
 * including char. For compile-time constant values (C++14+), validation can 
 * occur at compile time. For runtime values or C++11, validation occurs at runtime.
 * 
 * @tparam ToType Target type (must be numeric or char)
 * @tparam FromType Source type (must be numeric or char) 
 * @param value Value to cast
 * @return Safely cast value
 * @throws cast_exception if runtime validation fails
 * 
 * Usage: 
 *   auto result1 = numeric_cast<int>(42U);                  // Works in all standards
 *   constexpr auto result2 = numeric_cast<int>(42U);        // C++14+ compile-time validation
 */
template<typename ToType, typename FromType>
#if NCAST_HAS_CONSTEXPR_VALIDATION
NCAST_CONSTEXPR_14 ToType numeric_cast(FromType value) {
#else
ToType numeric_cast(FromType value) {
#endif
    return detail::numeric_cast_enhanced<ToType>(value);
}

/**
 * @brief Safe cast between char types only
 * 
 * This function template provides safe casting between char, signed char,
 * and unsigned char. It cannot be used with other numeric types.
 * char_cast is always safe and can be evaluated at compile time in C++14+.
 * 
 * @tparam ToType Target char type (char, signed char, unsigned char)
 * @tparam FromType Source char type (char, signed char, unsigned char)
 * @param value Value to cast
 * @return Safely cast value
 * 
 * Usage: 
 *   auto result = char_cast<unsigned char>('A');            // Works in all standards
 *   constexpr auto result2 = char_cast<unsigned char>('A'); // C++14+ compile-time
 */
template<typename ToType, typename FromType>
#if NCAST_HAS_CONSTEXPR_VALIDATION
NCAST_CONSTEXPR_14 ToType char_cast(FromType value) {
#else
ToType char_cast(FromType value) {
#endif
    return detail::char_cast_impl<ToType>(value, "unknown", 0, "unknown");
}

/**
 * @brief Macro version of numeric_cast with accurate location information
 * 
 * This macro captures file, line, and function information at the call site,
 * providing accurate location details in exception messages. Supports both
 * compile-time validation (C++14+) and runtime validation.
 * 
 * Usage: 
 *   auto result1 = NUMERIC_CAST(int, 42U);                 // Works in all standards
 *   constexpr auto result2 = NUMERIC_CAST(int, 42U);       // C++14+ compile-time validation
 */
#define NUMERIC_CAST(ToType, value) \
    ncast::detail::numeric_cast_enhanced<ToType>(value, __FILE__, __LINE__, __PRETTY_FUNCTION__)

/**
 * @brief Macro version of char_cast with accurate location information
 * 
 * This macro captures file, line, and function information at the call site,
 * providing accurate location details in exception messages. Supports both
 * compile-time evaluation (C++14+) and runtime execution.
 * 
 * Usage: 
 *   auto result1 = CHAR_CAST(unsigned char, 'A');          // Works in all standards
 *   constexpr auto result2 = CHAR_CAST(unsigned char, 'A'); // C++14+ compile-time
 */
#define CHAR_CAST(ToType, value) \
    ncast::detail::char_cast_impl<ToType>(value, __FILE__, __LINE__, __PRETTY_FUNCTION__)

} // namespace ncast

#endif // NCAST_H
