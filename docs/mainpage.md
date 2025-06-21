# ncast: Safe Numeric Casting for C++

ncast is a header-only C++ library for safe numeric casting between all integer and floating-point types, including char types. It provides:

- **number_cast**: Safe, checked conversions between all numeric types (including char)
- **char_cast**: Safe, reinterpret conversions between char, signed char, and unsigned char
- **Macro versions**: NUMBER_CAST and CHAR_CAST, which provide file/line info in exceptions
- **Exception safety**: Throws `ncast::cast_exception` on invalid conversions
- **Header-only**: Just include `<ncast/ncast.h>`

## Features
- Prevents silent overflows and narrowing errors
- Works with all standard integer and floating-point types
- Easy to use, minimal dependencies

## Example Usage
```cpp
#include <ncast/ncast.h>

int i = 42;
unsigned int u = number_cast<unsigned int>(i); // OK
unsigned int bad = number_cast<unsigned int>(-1); // throws ncast::cast_exception

char c = number_cast<char>(65); // 'A'
unsigned char uc = char_cast<unsigned char>('A'); // 65
```

## License
MIT License

---
For more details, see the API documentation and examples.
