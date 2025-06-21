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
 * - number_cast: Safe casting between all numeric types and char
 * - char_cast: Safe casting between signed/unsigned chars only
 * - Macro versions with accurate location information
 * - Optional validation (can be disabled with NCAST_DISABLE_VALIDATION)
 * 
 * @code
 * #include <ncast/ncast.h>
 * 
 * // Safe numeric casting
 * int value = 42;
 * unsigned int result = number_cast<unsigned int>(value);
 * 
 * // Safe char casting
 * signed char sc = 'A';
 * unsigned char uc = char_cast<unsigned char>(sc);
 * 
 * // Macro versions with location info
 * auto result2 = NUMBER_CAST(unsigned int, value);
 * auto result3 = CHAR_CAST(unsigned char, sc);
 * @endcode
 */

#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>
#include <limits>

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

// Macro to enable/disable validation
#ifndef NCAST_DISABLE_VALIDATION
#define NCAST_ENABLE_VALIDATION 1
#else
#define NCAST_ENABLE_VALIDATION 0
#endif

namespace detail {
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

    /**
     * @brief Helper function to perform safe numeric casting with validation
     */
    template<typename ToType, typename FromType>
    ToType number_cast_impl(FromType value, const char* file, int line, const char* function) {
        static_assert(is_numeric_or_char<ToType>::value, "ToType must be a numeric type or char");
        static_assert(is_numeric_or_char<FromType>::value, "FromType must be a numeric type or char");
        
#if !NCAST_ENABLE_VALIDATION
        // Suppress unused parameter warnings when validation is disabled
        (void)file;
        (void)line;
        (void)function;
#endif
        
#if NCAST_ENABLE_VALIDATION
        // Check for signed to unsigned conversion with negative value
        if (std::is_signed<FromType>::value && std::is_unsigned<ToType>::value) {
            if (value < 0) {
                std::ostringstream ss;
                ss << "Attempt to cast negative value (" << value 
                   << ") to unsigned type";
                throw cast_exception(ss.str(), file, line, function);
            }
        }
        
        // Check for range overflow/underflow  
        // Use careful comparison to avoid signed/unsigned comparison warnings
        if (static_cast<long long>(value) > static_cast<long long>(std::numeric_limits<ToType>::max())) {
            std::ostringstream ss;
            ss << "Value (" << value << ") exceeds maximum for target type ("
               << std::numeric_limits<ToType>::max() << ")";
            throw cast_exception(ss.str(), file, line, function);
        }
        
        if (static_cast<long long>(value) < static_cast<long long>(std::numeric_limits<ToType>::lowest())) {
            std::ostringstream ss;
            ss << "Value (" << value << ") is below minimum for target type ("
               << std::numeric_limits<ToType>::lowest() << ")";
            throw cast_exception(ss.str(), file, line, function);
        }
#endif
        return static_cast<ToType>(value);
    }

    /**
     * @brief Helper function to perform safe char casting with validation
     */
    template<typename ToType, typename FromType>
    ToType char_cast_impl(FromType value, const char* /* file */, int /* line */, const char* /* function */) {
        static_assert(is_char_type<ToType>::value, "ToType must be a char type (char, signed char, unsigned char)");
        static_assert(is_char_type<FromType>::value, "FromType must be a char type (char, signed char, unsigned char)");
        
        // char_cast is always safe between char types, no validation needed
        return static_cast<ToType>(value);
    }
}

/**
 * @brief Safe cast between numeric types and char with runtime validation
 * 
 * This function template provides safe casting between all numeric types
 * including char. It validates that the value is within the target type's range.
 * 
 * @tparam ToType Target type (must be numeric or char)
 * @tparam FromType Source type (must be numeric or char) 
 * @param value Value to cast
 * @return Safely cast value
 * @throws cast_exception if validation fails
 * 
 * Usage: auto result = number_cast<int>(unsigned_value);
 */
template<typename ToType, typename FromType>
ToType number_cast(FromType value) {
    return detail::number_cast_impl<ToType>(value, "unknown", 0, "unknown");
}

/**
 * @brief Safe cast between char types only
 * 
 * This function template provides safe casting between char, signed char,
 * and unsigned char. It cannot be used with other numeric types.
 * 
 * @tparam ToType Target char type (char, signed char, unsigned char)
 * @tparam FromType Source char type (char, signed char, unsigned char)
 * @param value Value to cast
 * @return Safely cast value
 * 
 * Usage: auto result = char_cast<unsigned char>(signed_char_value);
 */
template<typename ToType, typename FromType>
ToType char_cast(FromType value) {
    return detail::char_cast_impl<ToType>(value, "unknown", 0, "unknown");
}

/**
 * @brief Macro version of number_cast with accurate location information
 * 
 * This macro captures file, line, and function information at the call site,
 * providing accurate location details in exception messages.
 * 
 * Usage: auto result = NUMBER_CAST(int, unsigned_value);
 */
#define NUMBER_CAST(ToType, value) \
    ncast::detail::number_cast_impl<ToType>(value, __FILE__, __LINE__, __PRETTY_FUNCTION__)

/**
 * @brief Macro version of char_cast with accurate location information
 * 
 * This macro captures file, line, and function information at the call site,
 * providing accurate location details in exception messages.
 * 
 * Usage: auto result = CHAR_CAST(unsigned char, signed_char_value);
 */
#define CHAR_CAST(ToType, value) \
    ncast::detail::char_cast_impl<ToType>(value, __FILE__, __LINE__, __PRETTY_FUNCTION__)

} // namespace ncast

#endif // NCAST_H
