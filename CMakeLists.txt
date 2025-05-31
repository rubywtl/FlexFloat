cmake_minimum_required(VERSION 3.10)
project(FlexFloat)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create the library from your source file
add_library(CustomFP src/CustomFP.cpp)

# Make sure the library sees the headers
target_include_directories(CustomFP PUBLIC include)

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Your test executable
add_executable(fp_test test/unit_test.cpp)

# Link headers and libraries
target_include_directories(fp_test PRIVATE include)
target_link_libraries(fp_test PRIVATE gtest_main CustomFP)

# Discover and run tests
include(GoogleTest)
gtest_discover_tests(fp_test)
