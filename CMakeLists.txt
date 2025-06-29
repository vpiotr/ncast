cmake_minimum_required(VERSION 3.10)
project(ncast VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Enable comprehensive warning flags to match Windows behavior
if(MSVC)
    # Windows (MSVC) - Enable most warnings including conversion warnings
    add_compile_options(/W4 /WX- /wd4996)
else()
    # Linux/GCC or Clang - Enable comprehensive warnings including conversion warnings
    add_compile_options(
        -Wall                    # Enable most warnings
        -Wextra                  # Enable extra warnings
        -Wconversion             # Warn about type conversions that may alter values
        -Wsign-conversion        # Warn about sign conversions
        -Wfloat-conversion       # Warn about float conversions
        -Wcast-qual              # Warn about casts that remove qualifiers
        -Wcast-align             # Warn about casts that increase alignment requirements
        -Wshadow                 # Warn about variable shadowing
        -Wpedantic               # Warn about non-standard C++
        -Wunused                 # Warn about unused variables/functions
        -Wuninitialized          # Warn about uninitialized variables
    )
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/utest)

# Create interface library for header-only library
add_library(ncast INTERFACE)
target_include_directories(ncast INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Option to enable/disable validation
option(NCAST_DISABLE_RUNTIME_VALIDATION "Disable ncast runtime validation for performance" OFF)
if(NCAST_DISABLE_RUNTIME_VALIDATION)
    target_compile_definitions(ncast INTERFACE NCAST_DISABLE_RUNTIME_VALIDATION)
endif()

# Build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    # Modular test executables
    add_executable(test_ncast_core tests/test_ncast_core.cpp)
    target_link_libraries(test_ncast_core ncast)
    
    add_executable(test_ncast_int tests/test_ncast_int.cpp)
    target_link_libraries(test_ncast_int ncast)
    
    add_executable(test_ncast_float tests/test_ncast_float.cpp)
    target_link_libraries(test_ncast_float ncast)
    
    add_executable(test_ncast_char tests/test_ncast_char.cpp)
    target_link_libraries(test_ncast_char ncast)
    
    # Add tests to CTest
    add_test(NAME ncast_core_tests COMMAND test_ncast_core)
    add_test(NAME ncast_int_tests COMMAND test_ncast_int)
    add_test(NAME ncast_float_tests COMMAND test_ncast_float)
    add_test(NAME ncast_char_tests COMMAND test_ncast_char)
    
    # Set test properties
    set_tests_properties(ncast_core_tests ncast_int_tests ncast_float_tests ncast_char_tests PROPERTIES
        PASS_REGULAR_EXPRESSION "SUCCESS"
    )
endif()

# Build demos and benchmarks
option(BUILD_DEMOS "Build demonstration programs and benchmarks" ON)
if(BUILD_DEMOS)
    # Demo executable
    add_executable(demo_ncast demos/demo_ncast.cpp)
    target_link_libraries(demo_ncast ncast)
    
    # Benchmark no-validation module (compiled with NCAST_DISABLE_RUNTIME_VALIDATION)
    add_library(benchmark_ncast_no_validation demos/benchmark_ncast_no_validation.cpp)
    target_include_directories(benchmark_ncast_no_validation PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(benchmark_ncast_no_validation PRIVATE NCAST_DISABLE_RUNTIME_VALIDATION)
    
    # Benchmark executable (links with the no-validation module)
    add_executable(benchmark_ncast demos/benchmark_ncast.cpp)
    target_link_libraries(benchmark_ncast ncast benchmark_ncast_no_validation)
endif()

# Documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    option(BUILD_DOCS "Build documentation" ON)
    if(BUILD_DOCS)
        # Configure Doxygen
        set(DOXYGEN_INPUT_DIR ${CMAKE_SOURCE_DIR}/include)
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/docs)
        set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
        
        # Create Doxyfile
        configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in 
                       ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
        
        # Add documentation target
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# Installation
include(GNUInstallDirs)

# Install the header
install(FILES include/ncast/ncast.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ncast)

# Install the target
install(TARGETS ncast
        EXPORT ncastTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Create and install the config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/ncastConfig.cmake.in
    ${CMAKE_BINARY_DIR}/ncastConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ncast
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/ncastConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_BINARY_DIR}/ncastConfig.cmake
    ${CMAKE_BINARY_DIR}/ncastConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ncast
)

install(EXPORT ncastTargets
        FILE ncastTargets.cmake
        NAMESPACE ncast::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ncast
)

# CPack configuration
set(CPACK_PACKAGE_NAME "ncast")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Safe numeric casting header-only C++ library")
set(CPACK_PACKAGE_VENDOR "ncast")
set(CPACK_PACKAGE_CONTACT "ncast@example.com")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.md)

include(CPack)
