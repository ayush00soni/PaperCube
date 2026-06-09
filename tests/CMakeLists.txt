# Include the FetchContent module
include(FetchContent)

# Tell CMake where to download GoogleTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# Make GoogleTest available to our project
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # Required for Windows/MSVC compatibility
FetchContent_MakeAvailable(googletest)

# Create the test executable
add_executable(cube_test cube_test.cpp)

# Link your core engine AND the GoogleTest main library
target_link_libraries(cube_test PRIVATE 
    papercube_core 
    GTest::gtest_main # This provides the main() function automatically!
)

# Tell CMake's built-in testing tool (CTest) to discover our GTest macros
include(GoogleTest)
gtest_discover_tests(cube_test)