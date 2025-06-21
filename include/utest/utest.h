#ifndef __UTEST_H__
#define __UTEST_H__

/**
 * @file utest.h
 * @brief Simple and lightweight C++ unit testing library
 * @author Piotr Likus
 * @date 31/08/2020
 * 
 * @mainpage utest - Simple C++ Unit Testing Library
 * 
 * @section introduction Introduction
 * 
 * utest is a simple, lightweight, header-only C++ unit testing library that provides
 * essential testing functionality without external dependencies. It supports:
 * 
 * - Simple assertion macros for common test cases
 * - Test grouping and organization
 * - Performance timing for each test
 * - Unicode and ASCII checkmarks for test results
 * - Exception testing (throw/no-throw assertions)
 * - String and numeric comparisons
 * - String substring testing (contains/not contains)
 * - Comprehensive test reporting
 * 
 * @section usage_sec Basic Usage
 * 
 * @code{.cpp}
 * #include "utest.h"
 * 
 * // Define test functions
 * UTEST_FUNC_DEF(BasicMath) {
 *     UTEST_ASSERT_EQUALS(2 + 2, 4);
 *     UTEST_ASSERT_GT(5, 3);
 * }
 * 
 * UTEST_FUNC_DEF(StringTests) {
 *     UTEST_ASSERT_STR_EQUALS("hello", "hello");
 *     UTEST_ASSERT_STR_CONTAINS("hello world", "world");
 *     UTEST_ASSERT_STR_NOT_CONTAINS("success message", "error");
 * }
 * 
 * UTEST_FUNC_DEF2(Calculator, Addition) {
 *     UTEST_ASSERT_EQUALS(add(2, 3), 5);
 * }
 * 
 * int main() {
 *     UTEST_PROLOG();
 *     
 *     UTEST_FUNC(BasicMath);
 *     UTEST_FUNC(StringTests);
 *     UTEST_FUNC2(Calculator, Addition);
 *     
 *     UTEST_EPILOG();
 * }
 * @endcode
 * 
 * @section features_sec Features
 * 
 * - **Header-only**: Just include utest.h and start testing
 * - **No dependencies**: Works with standard C++ library only
 * - **Performance timing**: Built-in microsecond precision timing
 * - **Test grouping**: Organize related tests together
 * - **Rich assertions**: Support for various data types and conditions
 * - **Exception testing**: Test for expected exceptions or their absence
 * - **Flexible output**: ASCII or Unicode checkmarks, configurable verbosity
 * 
 * @section config_sec Configuration
 * 
 * The library can be configured using these macros in your test main():
 * - UTEST_USE_ASCII_CHECKMARKS() - Use [OK]/[FAIL] instead of checkmarks
 * - UTEST_SHOW_PERFORMANCE() - Show timing information for each test
 * - UTEST_ENABLE_VERBOSE_MODE() - Show test names before execution
 * - UTEST_ALLOW_EMPTY_TESTS() - Don't fail if no tests are run
 * 
 * By default, ASCII checkmarks and performance timing are enabled for
 * better compatibility and useful debugging information.
 */

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <type_traits>
#include <typeinfo>
#include <chrono>
#include <map>
#include <iomanip>

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

namespace utest {

/**
 * @defgroup exceptions Test Exceptions
 * @brief Exception classes for test failures
 * @{
 */

/**
 * @brief Exception thrown when an assertion fails
 * 
 * This exception provides detailed information about assertion failures,
 * including the source file, line number, and function where the failure occurred.
 * Used internally by assertion macros and can be caught in user code
 * for detailed error reporting.
 * 
 * @code{.cpp}
 * try {
 *     UTEST_ASSERT_TRUE(false);  // Will throw AssertionException
 * } catch (const utest::AssertionException& e) {
 *     std::cout << e.getFormattedMessage() << std::endl;
 * }
 * @endcode
 */
class AssertionException : public std::runtime_error {
public:
    /**
     * @brief Construct with simple message
     * @param message Error description
     */
    explicit AssertionException(const std::string& message) 
        : std::runtime_error(message), 
          file_("unknown"), 
          line_(0), 
          function_("unknown") {}
    
    /**
     * @brief Construct with full location information
     * @param message Error description
     * @param file Source file name
     * @param line Line number
     * @param function Function name
     */
    AssertionException(const std::string& message, const std::string& file, 
                      int line, const std::string& function)
        : std::runtime_error(message), 
          file_(file), 
          line_(line), 
          function_(function) {}
    
    virtual ~AssertionException() = default;
    
    /**
     * @brief Get the source file where the assertion failed
     * @return Source file name
     */
    const std::string& getFile() const { return file_; }
    
    /**
     * @brief Get the line number where the assertion failed
     * @return Line number
     */
    int getLine() const { return line_; }
    
    /**
     * @brief Get the function name where the assertion failed
     * @return Function name
     */
    const std::string& getFunction() const { return function_; }
    
    /**
     * @brief Get a complete formatted error message with location
     * @return Formatted error message including file, line, and function
     * 
     * @code{.cpp}
     * catch (const utest::AssertionException& e) {
     *     std::cout << e.getFormattedMessage() << std::endl;
     *     // Output: "Assertion failed at file.cpp:42 in test_function"
     * }
     * @endcode
     */
    std::string getFormattedMessage() const {
        std::ostringstream ss;
        ss << what() << " at " << file_ << ":" << line_ << " in " << function_;
        return ss.str();
    }
    
protected:
    std::string file_;      ///< Source file name
    int line_;              ///< Line number
    std::string function_;  ///< Function name
};

/** @} */ // end of exceptions group

// Forward declarations for string handling
namespace details {
    // SFINAE helper to detect if type is streamable
    template<typename T>
    class is_streamable {
        template<typename TT>
        static decltype(std::declval<std::ostringstream&>() << std::declval<TT>(), std::true_type()) test(int);
        template<typename>
        static std::false_type test(...);
    public:
        static const bool value = decltype(test<T>(0))::value;
    };

    // Helper to detect arithmetic types (but not bool or char types that might be better as strings)
    template<typename T>
    struct is_numeric : std::integral_constant<bool, 
        std::is_arithmetic<T>::value && 
        !std::is_same<T, bool>::value && 
        !std::is_same<T, char>::value &&
        !std::is_same<T, wchar_t>::value &&
        !std::is_same<T, char16_t>::value &&
        !std::is_same<T, char32_t>::value> {};

    // Helper to detect any pointer type or array type (ALL are disallowed in UTEST_ASSERT_EQUALS)
    // This includes string literals which decay to const char*
    template<typename T>
    struct is_any_pointer_or_array : std::integral_constant<bool, 
        std::is_pointer<T>::value ||
        std::is_array<T>::value ||
        std::is_pointer<typename std::decay<T>::type>::value ||
        std::is_member_pointer<T>::value ||
        (std::is_pointer<T>::value && std::is_function<typename std::remove_pointer<T>::type>::value)> {};

    // Compile-time assertion for pointer/array detection
    template<typename T, typename U>
    inline void check_not_pointer_types() {
        static_assert(!is_any_pointer_or_array<T>::value && !is_any_pointer_or_array<U>::value, 
                     "UTEST_ASSERT_EQUALS should not be used with pointers or string literals. Use UTEST_ASSERT_STR_EQUALS for string comparison or UTEST_ASSERT_PTR_EQUALS for pointer comparison.");
    }

    // C++11 compatible is_null_pointer implementation
    template<typename T>
    struct is_null_pointer_impl : std::false_type {};
    
    template<>
    struct is_null_pointer_impl<std::nullptr_t> : std::true_type {};
    
    template<typename T>
    struct is_null_pointer : is_null_pointer_impl<typename std::remove_cv<T>::type> {};

    // Helper to validate that only pointer types are used with UTEST_ASSERT_PTR_EQUALS
    template<typename T>
    struct is_valid_pointer : std::integral_constant<bool, 
        std::is_pointer<T>::value || 
        std::is_member_pointer<T>::value ||
        is_null_pointer<T>::value> {};

    // Compile-time assertion for pointer validation in UTEST_ASSERT_PTR_EQUALS
    template<typename T, typename U>
    inline void check_only_pointer_types() {
        static_assert(is_valid_pointer<T>::value && is_valid_pointer<U>::value, 
                     "UTEST_ASSERT_PTR_EQUALS can only be used with pointer types. Use UTEST_ASSERT_EQUALS for non-pointer comparisons.");
    }

    // For numeric types, use std::to_string
    template<typename T>
    inline typename std::enable_if<is_numeric<T>::value, std::string>::type
    convertToString(const T& value) {
        return std::to_string(value);
    }
    
    // For streamable non-numeric types, use ostringstream
    template<typename T>
    inline typename std::enable_if<!is_numeric<T>::value && is_streamable<T>::value, std::string>::type
    convertToString(const T& value) {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
    
    // For non-streamable types, return type information
    template<typename T>
    inline typename std::enable_if<!is_numeric<T>::value && !is_streamable<T>::value, std::string>::type
    convertToString(const T& value) {
        std::ostringstream ss;
        ss << "[" << typeid(T).name() << " at " << &value << "]";
        return ss.str();
    }
    
    // Specialization for std::string
    inline std::string convertToString(const std::string& value) {
        return value;
    }
    
    // Specialization for const char*
    inline std::string convertToString(const char* value) {
        return std::string(value);
    }
    
    // Specialization for bool
    inline std::string convertToString(bool value) {
        return value ? "true" : "false";
    }
    
    // Specialization for char
    inline std::string convertToString(char value) {
        return std::string(1, value);
    }

  // Helper for conversion to std::string (handles std::string and const char*)
  inline std::string to_string_for_str_assert(const std::string& s) { return s; }
  inline std::string to_string_for_str_assert(const char* s) { return std::string(s); }
  inline std::string to_string_for_str_assert(char* s) { return std::string(s); }
  
  // Wide string conversion with proper handling to avoid data loss warnings
  inline std::string to_string_for_str_assert(const std::wstring& s) { 
    std::string result;
    result.reserve(s.size());
    for (wchar_t wc : s) {
      // Explicitly cast to char with proper bounds checking
      if (wc <= 127) {
        result += static_cast<char>(wc);
      } else {
        result += '?'; // Replace non-ASCII characters with placeholder
      }
    }
    return result;
  }
  
  inline std::string to_string_for_str_assert(const wchar_t* s) { 
    if (s == nullptr) return std::string();
    return to_string_for_str_assert(std::wstring(s));
  }
  template<typename T>
  inline std::string to_string_for_str_assert(const T& s) { return convertToString(s); }

  // Helper for string contains check, always converts both arguments
  template<typename S1, typename S2>
  inline bool str_contains(const S1& str, const S2& substr) {
    return to_string_for_str_assert(str).find(to_string_for_str_assert(substr)) != std::string::npos;
  }

}

/**
 * @defgroup assertions Assertion Macros
 * @brief Macros for testing conditions and values
 * @{
 */

/**
 * @brief Convert a value to string representation for error messages
 * @param x Value to convert
 * @return String representation of the value
 * 
 * This macro uses SFINAE to automatically detect the best way to convert
 * a value to string, supporting standard types and custom streamable types.
 */
#define UTEST_TO_STRING( x ) utest::details::convertToString( ( x ) )

/**
 * @brief Assert that a condition is true
 * @param condition Boolean expression to test
 * 
 * Throws AssertionException if the condition evaluates to false.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_TRUE(value > 0);
 * UTEST_ASSERT_TRUE(ptr != nullptr);
 * @endcode
 */
#define UTEST_ASSERT_TRUE( condition )                             \
{                                                                   \
  if( !( condition ) )                                              \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "condition is false: '" << (#condition) << "'";           \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a condition is true with custom message
 * @param condition Boolean expression to test
 * @param msg Custom error message
 * 
 * Like UTEST_ASSERT_TRUE but with a custom error message.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_TRUE_MSG(value > 0, "Value must be positive");
 * @endcode
 */
#define UTEST_ASSERT_TRUE_MSG( condition, msg )                    \
{                                                                   \
  if( !( condition ) )                                              \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "assertion failed, '" << msg << "'";                      \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a condition is false
 * @param condition Boolean expression to test
 * 
 * Throws AssertionException if the condition evaluates to true.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_FALSE(value == 0);
 * UTEST_ASSERT_FALSE(list.empty());
 * @endcode
 */
#define UTEST_ASSERT_FALSE( condition )                            \
{                                                                   \
  if( ( condition ) )                                               \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "condition is true: '" << (#condition) << "'";            \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a condition is false with custom message
 * @param condition Boolean expression to test
 * @param msg Custom error message
 * 
 * Like UTEST_ASSERT_FALSE but with a custom error message.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_FALSE_MSG(value == 0, "Value should not be zero");
 * @endcode
 */
#define UTEST_ASSERT_FALSE_MSG( condition, msg )                   \
{                                                                   \
  if( ( condition ) )                                               \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "assertion failed, '" << msg << "'";                      \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that two values are equal
 * @param x First value
 * @param y Second value
 * 
 * Uses operator!= to compare values. Supports any type that can be
 * converted to string for error messages.
 * 
 * Note: Do not use with pointers. Use UTEST_ASSERT_PTR_EQUALS for pointer comparison.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_EQUALS(calculator.add(2, 3), 5);
 * UTEST_ASSERT_EQUALS(std::string("hello"), "hello");
 * @endcode
 */
#define UTEST_ASSERT_EQUALS( x, y )                                \
{                                                                   \
  utest::details::check_not_pointer_types<decltype(x), decltype(y)>(); \
  if( ( x ) != ( y ) )                                              \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed: " << UTEST_TO_STRING( ( x ) )          \
       << " != " << UTEST_TO_STRING( ( y ) );                       \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that two values are equal with custom message
 * @param x First value
 * @param y Second value
 * @param msg Custom error message
 * 
 * Like UTEST_ASSERT_EQUALS but with a custom error message.
 * Note: Do not use with pointers. Use UTEST_ASSERT_PTR_EQUALS_MSG for pointer comparison.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_EQUALS_MSG(result, expected, "Calculation result mismatch");
 * @endcode
 */
#define UTEST_ASSERT_EQUALS_MSG( x, y, msg )                       \
{                                                                   \
  utest::details::check_not_pointer_types<decltype(x), decltype(y)>(); \
  if( ( x ) != ( y ) )                                              \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': "                     \
       << UTEST_TO_STRING( ( x ) ) << " != " << UTEST_TO_STRING( ( y ) ); \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that two pointers are equal
 * @param ptr1 First pointer
 * @param ptr2 Second pointer
 * 
 * Compares pointer addresses directly. Use this for pointer comparison
 * instead of UTEST_ASSERT_EQUALS.
 * 
 * @code{.cpp}
 * int* p1 = &value;
 * int* p2 = &value;
 * UTEST_ASSERT_PTR_EQUALS(p1, p2);
 * @endcode
 */
#define UTEST_ASSERT_PTR_EQUALS( ptr1, ptr2 )                      \
{                                                                   \
  utest::details::check_only_pointer_types<decltype(ptr1), decltype(ptr2)>(); \
  if( ( ptr1 ) != ( ptr2 ) )                                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Pointer assertion failed: " << reinterpret_cast<const void*>( ptr1 ) \
       << " != " << reinterpret_cast<const void*>( ptr2 );               \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that two pointers are equal with custom message
 * @param ptr1 First pointer
 * @param ptr2 Second pointer
 * @param msg Custom error message
 * 
 * Like UTEST_ASSERT_PTR_EQUALS but with a custom error message.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_PTR_EQUALS_MSG(p1, p2, "Pointers should point to same object");
 * @endcode
 */
#define UTEST_ASSERT_PTR_EQUALS_MSG( ptr1, ptr2, msg )             \
{                                                                   \
  utest::details::check_only_pointer_types<decltype(ptr1), decltype(ptr2)>(); \
  if( ( ptr1 ) != ( ptr2 ) )                                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Pointer assertion failed, '" << msg << "': "             \
       << reinterpret_cast<const void*>( ptr1 ) << " != " << reinterpret_cast<const void*>( ptr2 ); \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that two pointers are not equal
 * @param ptr1 First pointer
 * @param ptr2 Second pointer
 * 
 * Throws AssertionException if the pointers are equal.
 * 
 * @code{.cpp}
 * int* p1 = &value1;
 * int* p2 = &value2;
 * UTEST_ASSERT_PTR_NOT_EQUALS(p1, p2);
 * @endcode
 */
#define UTEST_ASSERT_PTR_NOT_EQUALS( ptr1, ptr2 )                  \
{                                                                   \
  if( ( ptr1 ) == ( ptr2 ) )                                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Pointer assertion failed: " << reinterpret_cast<const void*>( ptr1 ) \
       << " == " << reinterpret_cast<const void*>( ptr2 );          \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that two pointers are not equal with custom message
 * @param ptr1 First pointer
 * @param ptr2 Second pointer
 * @param msg Custom error message
 * 
 * Like UTEST_ASSERT_PTR_NOT_EQUALS but with a custom error message.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_PTR_NOT_EQUALS_MSG(p1, p2, "Pointers should be different");
 * @endcode
 */
#define UTEST_ASSERT_PTR_NOT_EQUALS_MSG( ptr1, ptr2, msg )         \
{                                                                   \
  if( ( ptr1 ) == ( ptr2 ) )                                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Pointer assertion failed, '" << msg << "': "             \
       << reinterpret_cast<const void*>( ptr1 ) << " == " << reinterpret_cast<const void*>( ptr2 ); \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a pointer is null
 * @param ptr Pointer to test
 * 
 * Throws AssertionException if the pointer is not null.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_NULL(ptr);
 * UTEST_ASSERT_NULL(object.getOptionalResource());
 * @endcode
 */
#define UTEST_ASSERT_NULL( ptr )                                   \
{                                                                   \
  if( ( ptr ) != nullptr )                                          \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, pointer is not null: " << (#ptr);      \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a pointer is not null
 * @param ptr Pointer to test
 * 
 * Throws AssertionException if the pointer is null.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_NOT_NULL(ptr);
 * UTEST_ASSERT_NOT_NULL(factory.createObject());
 * @endcode
 */
#define UTEST_ASSERT_NOT_NULL( ptr )                               \
{                                                                   \
  if( ( ptr ) == nullptr )                                          \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, pointer is null: '" << (#ptr) << "'";  \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

// New assertion macros

#define UTEST_ASSERT_NOT_EQUALS( x, y )                            \
{                                                                   \
  if( ( x ) == ( y ) )                                              \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed: " << UTEST_TO_STRING( ( x ) )          \
       << " == " << UTEST_TO_STRING( ( y ) );                       \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_STR_EQUALS( x, y )                            \
{                                                                   \
  if( std::string( x ) != std::string( y ) )                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "String assertion failed: \"" << std::string( x )         \
       << "\" != \"" << std::string( y ) << "\"";                   \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_STR_NOT_EQUALS( x, y )                        \
{                                                                   \
  if( std::string( x ) == std::string( y ) )                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "String assertion failed: \"" << std::string( x )         \
       << "\" == \"" << std::string( y ) << "\"";                   \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a string contains a substring
 * @param str String to search in
 * @param substr Substring to search for
 * 
 * Throws AssertionException if the string does not contain the substring.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_STR_CONTAINS("Hello world", "world");
 * UTEST_ASSERT_STR_CONTAINS(myString, "important text");
 * @endcode
 */

#define UTEST_ASSERT_STR_CONTAINS( a_string, a_substr )                                   \
{                                                                                  \
  if( !utest::details::str_contains(a_string, a_substr) )                                 \
  {                                                                                \
    std::string _utest_str = utest::details::to_string_for_str_assert(a_string);        \
    std::string _utest_substr = utest::details::to_string_for_str_assert(a_substr);  \
    std::ostringstream ss;                                                         \
    ss << "String assertion failed: \"" << _utest_str                              \
      << "\" does not contain \"" << _utest_substr << "\"";                       \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                                \
}

/**
 * @brief Assert that a string does not contain a substring
 * @param str String to search in
 * @param substr Substring to search for
 * 
 * Throws AssertionException if the string contains the substring.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_STR_NOT_CONTAINS("Hello world", "error");
 * UTEST_ASSERT_STR_NOT_CONTAINS(response, "failure");
 * @endcode
 */
#define UTEST_ASSERT_STR_NOT_CONTAINS( a_string, a_substr )               \
{                                                                   \
  if( utest::details::str_contains(a_string, a_substr) )                                 \
  {                                                                 \
    std::string _utest_str = utest::details::to_string_for_str_assert(a_string);        \
    std::string _utest_substr = utest::details::to_string_for_str_assert(a_substr);      \
    std::ostringstream ss;                                          \
    ss << "String assertion failed: \"" << _utest_str               \
       << "\" contains \"" << _utest_substr << "\"";                \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_GT( x, y )                                    \
{                                                                   \
  if( !( ( x ) > ( y ) ) )                                          \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed: " << UTEST_TO_STRING( ( x ) )          \
       << " is not greater than " << UTEST_TO_STRING( ( y ) );      \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_GTE( x, y )                                   \
{                                                                   \
  if( !( ( x ) >= ( y ) ) )                                         \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed: " << UTEST_TO_STRING( ( x ) )          \
       << " is not greater than or equal to " << UTEST_TO_STRING( ( y ) ); \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_LT( x, y )                                    \
{                                                                   \
  if( !( ( x ) < ( y ) ) )                                          \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed: " << UTEST_TO_STRING( ( x ) )          \
       << " is not less than " << UTEST_TO_STRING( ( y ) );         \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_LTE( x, y )                                   \
{                                                                   \
  if( !( ( x ) <= ( y ) ) )                                         \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed: " << UTEST_TO_STRING( ( x ) )          \
       << " is not less than or equal to " << UTEST_TO_STRING( ( y ) ); \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

// MSG versions for all assertion macros

#define UTEST_ASSERT_NOT_EQUALS_MSG( x, y, msg )                   \
{                                                                   \
  if( ( x ) == ( y ) )                                              \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': "                     \
       << UTEST_TO_STRING( ( x ) ) << " == " << UTEST_TO_STRING( ( y ) ); \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_STR_EQUALS_MSG( x, y, msg )                   \
{                                                                   \
  if( std::string( x ) != std::string( y ) )                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "String assertion failed, '" << msg << "': \""            \
       << std::string( x ) << "\" != \"" << std::string( y ) << "\""; \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_STR_NOT_EQUALS_MSG( x, y, msg )               \
{                                                                   \
  if( std::string( x ) == std::string( y ) )                       \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "String assertion failed, '" << msg << "': \""            \
       << std::string( x ) << "\" == \"" << std::string( y ) << "\""; \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a string contains a substring with custom message
 * @param str String to search in
 * @param substr Substring to search for
 * @param msg Custom error message
 * 
 * Like UTEST_ASSERT_STR_CONTAINS but with a custom error message.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_STR_CONTAINS_MSG(response, "success", "Response should indicate success");
 * @endcode
 */
#define UTEST_ASSERT_STR_CONTAINS_MSG( a_string, a_substr, a_msg )          \
{                                                                   \
  if( !utest::details::str_contains(a_string, a_substr))         \
  {                                                                 \
    std::string _utest_str = utest::details::to_string_for_str_assert(a_string);        \
    std::string _utest_substr = utest::details::to_string_for_str_assert(a_substr);      \
    std::ostringstream ss;                                          \
    ss << "String assertion failed, '" << a_msg << "': \""            \
       << _utest_str << "\" does not contain \"" << _utest_substr << "\""; \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @brief Assert that a string does not contain a substring with custom message
 * @param str String to search in
 * @param substr Substring to search for
 * @param msg Custom error message
 * 
 * Like UTEST_ASSERT_STR_NOT_CONTAINS but with a custom error message.
 * 
 * @code{.cpp}
 * UTEST_ASSERT_STR_NOT_CONTAINS_MSG(response, "error", "Response should not contain errors");
 * @endcode
 */
#define UTEST_ASSERT_STR_NOT_CONTAINS_MSG( a_string, a_substr, a_msg )      \
{                                                                   \
  if( utest::details::str_contains(a_string, a_substr) )         \
  {                                                                 \
    std::string _utest_str = utest::details::to_string_for_str_assert(a_string);        \
    std::string _utest_substr = utest::details::to_string_for_str_assert(a_substr);      \
    std::ostringstream ss;                                          \
    ss << "String assertion failed, '" << a_msg << "': \""            \
       << _utest_str << "\" contains \"" << _utest_substr << "\"";  \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_GT_MSG( x, y, msg )                          \
{                                                                   \
  if( !( ( x ) > ( y ) ) )                                         \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': "                     \
       << UTEST_TO_STRING( ( x ) ) << " is not greater than or equal to " \
       << UTEST_TO_STRING( ( y ) );                                 \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_GTE_MSG( x, y, msg )                          \
{                                                                   \
  if( !( ( x ) >= ( y ) ) )                                         \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': "                     \
       << UTEST_TO_STRING( ( x ) ) << " is not greater than or equal to " \
       << UTEST_TO_STRING( ( y ) );                                 \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_LT_MSG( x, y, msg )                           \
{                                                                   \
  if( !( ( x ) < ( y ) ) )                                          \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': "                     \
       << UTEST_TO_STRING( ( x ) ) << " is not less than "          \
       << UTEST_TO_STRING( ( y ) );                                 \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_LTE_MSG( x, y, msg )                          \
{                                                                   \
  if( !( ( x ) <= ( y ) ) )                                         \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': "                     \
       << UTEST_TO_STRING( ( x ) ) << " is not less than or equal to " \
       << UTEST_TO_STRING( ( y ) );                                 \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_NULL_MSG( ptr, msg )                          \
{                                                                   \
  if( ( ptr ) != nullptr )                                          \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': pointer is not null: " << (#ptr); \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

#define UTEST_ASSERT_NOT_NULL_MSG( ptr, msg )                      \
{                                                                   \
  if( ( ptr ) == nullptr )                                          \
  {                                                                 \
    std::ostringstream ss;                                          \
    ss << "Assertion failed, '" << msg << "': pointer is null: '" << (#ptr) << "'"; \
    throw utest::AssertionException(ss.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
  }                                                                 \
}

/**
 * @defgroup aliases Convenient Aliases
 * @brief Short aliases for commonly used assertion macros
 * @{
 */

/**
 * @brief Short alias for UTEST_ASSERT_EQUALS
 * @param x First value
 * @param y Second value
 */
#define UTEST_ASSERT_EQ( x, y ) UTEST_ASSERT_EQUALS( x, y )

/**
 * @brief Short alias for UTEST_ASSERT_NOT_EQUALS
 * @param x First value  
 * @param y Second value
 */
#define UTEST_ASSERT_NEQ( x, y ) UTEST_ASSERT_NOT_EQUALS( x, y )

/**
 * @brief Short alias for UTEST_ASSERT_STR_EQUALS
 * @param x First string
 * @param y Second string
 */
#define UTEST_ASSERT_SEQ( x, y ) UTEST_ASSERT_STR_EQUALS( x, y )

/**
 * @brief Short alias for UTEST_ASSERT_STR_NOT_EQUALS
 * @param x First string
 * @param y Second string
 */
#define UTEST_ASSERT_SNEQ( x, y ) UTEST_ASSERT_STR_NOT_EQUALS( x, y )

/**
 * @brief Short alias for UTEST_ASSERT_STR_CONTAINS
 * @param str String to search in
 * @param substr Substring to search for
 */
#define UTEST_ASSERT_SC( str, substr ) UTEST_ASSERT_STR_CONTAINS( str, substr )

/**
 * @brief Short alias for UTEST_ASSERT_STR_NOT_CONTAINS
 * @param str String to search in
 * @param substr Substring to search for
 */
#define UTEST_ASSERT_SNC( str, substr ) UTEST_ASSERT_STR_NOT_CONTAINS( str, substr )

/**
 * @brief Short alias for UTEST_ASSERT_STR_CONTAINS
 * @param str String to search in
 * @param substr Substring to search for
 */
#define UTEST_ASSERT_SC( str, substr ) UTEST_ASSERT_STR_CONTAINS( str, substr )

/**
 * @brief Short alias for UTEST_ASSERT_STR_NOT_CONTAINS
 * @param str String to search in
 * @param substr Substring to search for
 */
#define UTEST_ASSERT_SNC( str, substr ) UTEST_ASSERT_STR_NOT_CONTAINS( str, substr )

/** @} */ // end of aliases group

namespace details {

    // Test tracking structure
    struct TestResult {
        std::string name;
        std::string group; // For grouping tests by first parameter
        bool passed;
        std::string error;
        double elapsedTime; // in milliseconds
    };
    
    // Global test results collection
    inline std::vector<TestResult>& getTestResults() {
        static std::vector<TestResult> results;
        return results;
    }
    
    // Configuration for handling no-tests scenario
    inline bool& getAllowEmptyTests() {
        static bool allowEmpty = false;
        return allowEmpty;
    }
    
    // Configuration for ASCII vs Unicode checkmarks
    inline bool& getUseAsciiCheckmarks() {
        static bool useAscii = true;  // Default to ASCII for better compatibility
        return useAscii;
    }
    
    // Configuration for showing performance information
    inline bool& getShowPerformanceInfo() {
        static bool showPerf = true;  // Default to showing performance info
        return showPerf;
    }
    
    // Configuration for verbose mode (showing test names before execution)
    inline bool& getVerboseMode() {
        static bool verbose = false;  // Default to non-verbose
        return verbose;
    }
    
    template<typename Func>
    void testFunc(const char *name, Func f, bool &failed) {
        TestResult result;
        result.name = name;
        result.name = name;
        result.group = ""; // No group for single tests
        result.passed = true;
        result.elapsedTime = 0.0;
        
        // Get checkmark symbols
        const char* successMark = getUseAsciiCheckmarks() ? "[OK]" : "✓";
        const char* failMark = getUseAsciiCheckmarks() ? "[FAIL]" : "✗";
        
        // Show test name before execution if verbose mode is enabled
        if (getVerboseMode()) {
            std::cout << "Running test: " << std::string(name) << "\n";
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            f();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            result.elapsedTime = static_cast<double>(duration.count()) / 1000.0; // Convert to milliseconds
            
            std::cout << successMark << " Test [" << std::string(name) << "] succeeded";
            if (getShowPerformanceInfo()) {
                std::cout << " (" << std::fixed << std::setprecision(3) << result.elapsedTime << "ms)";
            }
            std::cout << "\n";
        }
        catch (const AssertionException &e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            result.elapsedTime = static_cast<double>(duration.count()) / 1000.0;
            
            std::cout << failMark << " Test [" << std::string(name) << "] failed!, error: " << e.getFormattedMessage();
            if (getShowPerformanceInfo()) {
                std::cout << " (" << std::fixed << std::setprecision(3) << result.elapsedTime << "ms)";
            }
            std::cout << "\n";
            failed = true;
            result.passed = false;
            result.error = e.getFormattedMessage();
        }
        catch (std::exception &e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            result.elapsedTime = static_cast<double>(duration.count()) / 1000.0;
            
            std::cout << failMark << " Test [" << std::string(name) << "] failed with unexpected exception!, error: " << e.what();
            if (getShowPerformanceInfo()) {
                std::cout << " (" << std::fixed << std::setprecision(3) << result.elapsedTime << "ms)";
            }
            std::cout << "\n";
            failed = true;
            result.passed = false;
            result.error = e.what();
        }
        
        getTestResults().push_back(result);
    }

    // Overloaded version for grouped tests (UTEST_FUNC2)
    template<typename Func>
    void testFunc2(const char *group, const char *name, Func f, bool &failed) {
        TestResult result;
        result.name = name;
        result.group = group;
        result.passed = true;
        result.elapsedTime = 0.0;
        
        // Get checkmark symbols
        const char* successMark = getUseAsciiCheckmarks() ? "[OK]" : "✓";
        const char* failMark = getUseAsciiCheckmarks() ? "[FAIL]" : "✗";
        
        // Show test name before execution if verbose mode is enabled
        if (getVerboseMode()) {
            std::cout << "Running test: " << std::string(group) << "::" << std::string(name) << "\n";
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            f();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            result.elapsedTime = static_cast<double>(duration.count()) / 1000.0; // Convert to milliseconds
            
            std::cout << successMark << " Test [" << std::string(group) << "::" << std::string(name) << "] succeeded";
            if (getShowPerformanceInfo()) {
                std::cout << " (" << std::fixed << std::setprecision(3) << result.elapsedTime << "ms)";
            }
            std::cout << "\n";
        }
        catch (const AssertionException &e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            result.elapsedTime = static_cast<double>(duration.count()) / 1000.0;
            
            std::cout << failMark << " Test [" << std::string(group) << "::" << std::string(name) << "] failed!, error: " << e.getFormattedMessage();
            if (getShowPerformanceInfo()) {
                std::cout << " (" << std::fixed << std::setprecision(3) << result.elapsedTime << "ms)";
            }
            std::cout << "\n";
            failed = true;
            result.passed = false;
            result.error = e.getFormattedMessage();
        }
        catch (std::exception &e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            result.elapsedTime = static_cast<double>(duration.count()) / 1000.0;
            
            std::cout << failMark << " Test [" << std::string(group) << "::" << std::string(name) << "] failed with unexpected exception!, error: " << e.what();
            if (getShowPerformanceInfo()) {
                std::cout << " (" << std::fixed << std::setprecision(3) << result.elapsedTime << "ms)";
            }
            std::cout << "\n";
            failed = true;
            result.passed = false;
            result.error = e.what();
        }
        
        getTestResults().push_back(result);
    }

    template<typename Func>
    inline void AssertThrows(Func assertion, const std::string &msg = "") {
        bool throwFound = false;
        std::string exceptionMsg;
        try {
            assertion();
        }
        catch (const std::exception& e) {
            throwFound = true;
            exceptionMsg = e.what();
        }
        catch (...) {
            throwFound = true;
            exceptionMsg = "unknown exception";
        }

        if (!throwFound) {
            if (msg.size() > 0) {
                std::ostringstream ss;
                ss << "Expected exception was not thrown: " << msg;
                throw utest::AssertionException(ss.str());
            } else {
                throw utest::AssertionException("Expected exception was not thrown");
            }
        }
    }

    template<typename Func>
    inline void AssertDoesNotThrow(Func assertion, const std::string &msg = "") {
        try {
            assertion();
        }
        catch (const std::exception& e) {
            if (msg.size() > 0) {
                std::ostringstream ss;
                ss << "Unexpected exception thrown: " << msg << " - " << e.what();
                throw utest::AssertionException(ss.str());
            } else {
                std::ostringstream ss;
                ss << "Unexpected exception thrown: " << e.what();
                throw utest::AssertionException(ss.str());
            }
        }
        catch (...) {
            if (msg.size() > 0) {
                std::ostringstream ss;
                ss << "Unexpected unknown exception thrown: " << msg;
                throw utest::AssertionException(ss.str());
            } else {
                throw utest::AssertionException("Unexpected unknown exception thrown");
            }
        }
    }
} // namespace details

#define UTEST_ASSERT_THROWS(F) utest::details::AssertThrows(F)

#define UTEST_ASSERT_THROWS_MSG(F, MSG) utest::details::AssertThrows(F, std::string(MSG))

#define UTEST_ASSERT_DOES_NOT_THROW(F) utest::details::AssertDoesNotThrow(F)

#define UTEST_ASSERT_DOES_NOT_THROW_MSG(F, MSG) utest::details::AssertDoesNotThrow(F, std::string(MSG))

/** @} */ // end of assertions group

/**
 * @defgroup test_execution Test Execution Macros
 * @brief Macros for defining and running tests
 * @{
 */

/**
 * @brief Execute a simple test function
 * @param a Test function name (without test_ prefix)
 * 
 * Executes a test function defined with UTEST_FUNC_DEF(a).
 * The function will be called as test_##a().
 * 
 * @code{.cpp}
 * UTEST_FUNC_DEF(MyTest) {
 *     UTEST_ASSERT_TRUE(true);
 * }
 * 
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_FUNC(MyTest);  // Calls test_MyTest()
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_FUNC(a) utest::details::testFunc(#a, test_##a, errorFound)

/**
 * @brief Initialize the test framework
 * 
 * Must be called at the beginning of main() before any test execution.
 * Initializes error tracking and clears any previous test results.
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();  // Always call first
 *     // ... run tests ...
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_PROLOG() bool errorFound = false; \
    utest::details::getTestResults().clear()

/**
 * @brief Allow tests to run even if no test functions are executed
 * 
 * By default, running zero tests is considered a failure. This macro
 * allows empty test runs to succeed, useful for conditional testing.
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_ALLOW_EMPTY_TESTS();  // Won't fail if no tests run
 *     // conditionally run tests...
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_ALLOW_EMPTY_TESTS() utest::details::getAllowEmptyTests() = true

/**
 * @brief Enable ASCII checkmarks instead of Unicode symbols
 * 
 * Changes test output from ✓/✗ to [OK]/[FAIL] for better compatibility
 * with terminals that don't support Unicode. This is enabled by default.
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_USE_ASCII_CHECKMARKS();  // Use [OK]/[FAIL]
 *     // ... run tests ...
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_USE_ASCII_CHECKMARKS() utest::details::getUseAsciiCheckmarks() = true

/**
 * @brief Enable Unicode checkmarks for test results
 * 
 * Changes test output from [OK]/[FAIL] to ✓/✗ for nicer formatting
 * on terminals that support Unicode characters.
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_USE_UNICODE_CHECKMARKS();  // Use ✓/✗
 *     // ... run tests ...
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_USE_UNICODE_CHECKMARKS() utest::details::getUseAsciiCheckmarks() = false

/**
 * @brief Enable performance timing information in test output
 * 
 * Shows execution time for each test in milliseconds.
 * This is enabled by default.
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_SHOW_PERFORMANCE();  // Show timing info
 *     // ... run tests ...
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_SHOW_PERFORMANCE() utest::details::getShowPerformanceInfo() = true

/**
 * @brief Disable performance timing information in test output
 * 
 * Hides execution time information to make output cleaner.
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_HIDE_PERFORMANCE();  // Hide timing info
 *     // ... run tests ...
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_HIDE_PERFORMANCE() utest::details::getShowPerformanceInfo() = false

/**
 * @brief Enable verbose mode to show test names before execution
 * 
 * Shows the name of each test before it runs, useful for debugging
 * and understanding test execution flow.
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_ENABLE_VERBOSE_MODE();  // Show test names before execution
 *     // ... run tests ...
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_ENABLE_VERBOSE_MODE() utest::details::getVerboseMode() = true

/** @} */ // end of test_execution group

/**
 * @defgroup test_definition Test Definition Macros
 * @brief Macros for defining test functions
 * @{
 */

/**
 * @brief Define a simple test function
 * @param a Test name
 * 
 * Creates a test function named test_##a() that can be executed with UTEST_FUNC(a).
 * 
 * @code{.cpp}
 * UTEST_FUNC_DEF(BasicArithmetic) {
 *     UTEST_ASSERT_EQUALS(2 + 2, 4);
 * }
 * 
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_FUNC(BasicArithmetic);
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_FUNC_DEF(a) void test_##a()

/**
 * @brief Define a grouped test function
 * @param a Group name (e.g., class or module name)
 * @param b Test name within the group
 * 
 * Creates a test function named test_##a##_##b() that can be executed with UTEST_FUNC2(a, b).
 * Tests with the same group name will be displayed together in the test summary.
 * 
 * @code{.cpp}
 * UTEST_FUNC_DEF2(Calculator, Addition) {
 *     UTEST_ASSERT_EQUALS(calc.add(2, 3), 5);
 * }
 * 
 * UTEST_FUNC_DEF2(Calculator, Subtraction) {
 *     UTEST_ASSERT_EQUALS(calc.subtract(5, 3), 2);
 * }
 * 
 * int main() {
 *     UTEST_PROLOG();
 *     UTEST_FUNC2(Calculator, Addition);
 *     UTEST_FUNC2(Calculator, Subtraction);
 *     UTEST_EPILOG();
 * }
 * @endcode
 */
#define UTEST_FUNC_DEF2(a, b) void test_##a##_##b()

/** @} */ // end of test_definition group

/**
 * @defgroup test_runner Test Runner Macros
 * @brief Macros for executing tests and managing test flow
 * @{
 */

/**
 * @brief Execute a grouped test function
 * @param a Group name
 * @param b Test name within the group
 * 
 * Executes a test function defined with UTEST_FUNC_DEF2(a, b).
 * The test will be displayed as "Group::Test" in the output and grouped
 * with other tests that have the same group name.
 * 
 * @code{.cpp}
 * UTEST_FUNC2(Calculator, Addition);  // Runs test_Calculator_Addition()
 * @endcode
 */
#define UTEST_FUNC2(a, b) utest::details::testFunc2(#a, #b, test_##a##_##b, errorFound)

/**
 * @brief Finalize testing and display results
 * 
 * Must be called at the end of main() to display test summary and return
 * appropriate exit code. Returns EXIT_SUCCESS if all tests passed,
 * EXIT_FAILURE if any test failed or no tests were run (unless allowed).
 * 
 * The summary includes:
 * - Individual test results with checkmarks
 * - Grouped display for UTEST_FUNC2 tests
 * - Performance timing (if enabled)
 * - Overall statistics
 * - Final SUCCESS/FAILURE status
 * 
 * @code{.cpp}
 * int main() {
 *     UTEST_PROLOG();
 *     // ... run tests ...
 *     UTEST_EPILOG();  // Always call last, returns exit code
 * }
 * @endcode
 */
#define UTEST_EPILOG() do { \
    std::cout << "\n======================================\n"; \
    std::cout << "Test Summary:\n"; \
    std::cout << "======================================\n"; \
    auto& results = utest::details::getTestResults(); \
    int passed = 0, failed = 0; \
    double totalTime = 0.0; \
    const char* successMark = utest::details::getUseAsciiCheckmarks() ? "[OK]" : "✓"; \
    const char* failMark = utest::details::getUseAsciiCheckmarks() ? "[FAIL]" : "✗"; \
    if (results.empty()) { \
        std::cout << "No tests were run!\n"; \
        std::cout << "======================================\n"; \
        if (utest::details::getAllowEmptyTests()) { \
            std::cout << "SUCCESS (empty tests allowed)\n"; \
            return EXIT_SUCCESS; \
        } else { \
            std::cout << "FAILURE\n"; \
            return EXIT_FAILURE; \
        } \
    } \
    std::map<std::string, std::vector<const utest::details::TestResult*>> groupedResults; \
    for (const auto& result : results) { \
        if (result.group.empty()) { \
            groupedResults[""].push_back(&result); \
        } else { \
            groupedResults[result.group].push_back(&result); \
        } \
        totalTime += result.elapsedTime; \
    } \
    for (const auto& group : groupedResults) { \
        if (!group.first.empty()) { \
            std::cout << "\n" << group.first << ":\n"; \
        } \
        for (const auto* result : group.second) { \
            if (result->passed) { \
                std::cout << successMark << " "; \
                if (!result->group.empty()) { \
                    std::cout << result->name; \
                } else { \
                    std::cout << result->name; \
                } \
                if (utest::details::getShowPerformanceInfo()) { \
                    std::cout << " (" << std::fixed << std::setprecision(3) << result->elapsedTime << "ms)"; \
                } \
                std::cout << "\n"; \
                passed++; \
            } else { \
                std::cout << failMark << " "; \
                if (!result->group.empty()) { \
                    std::cout << result->name; \
                } else { \
                    std::cout << result->name; \
                } \
                std::cout << " - " << result->error; \
                if (utest::details::getShowPerformanceInfo()) { \
                    std::cout << " (" << std::fixed << std::setprecision(3) << result->elapsedTime << "ms)"; \
                } \
                std::cout << "\n"; \
                failed++; \
            } \
        } \
    } \
    std::cout << "--------------------------------------\n"; \
    std::cout << "Total: " << (passed + failed) << " tests, "; \
    std::cout << passed << " passed " << successMark << ", " << failed << " failed " << failMark; \
    if (utest::details::getShowPerformanceInfo()) { \
        std::cout << " (Total time: " << std::fixed << std::setprecision(3) << totalTime << "ms)"; \
    } \
    std::cout << "\n"; \
    std::cout << "======================================\n"; \
    if (errorFound || failed > 0) { \
        std::cout << "FAILURE\n"; \
        return EXIT_FAILURE; \
    } else { \
        std::cout << "SUCCESS\n"; \
        return EXIT_SUCCESS; \
    } \
} while(0)

/** @} */ // end of test_runner group

} // namespace utest

#endif
