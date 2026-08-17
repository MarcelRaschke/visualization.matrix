# Tests for visualization.matrix
#
# Included via add_subdirectory(tests) from the top-level CMakeLists.txt when
# BUILD_TESTING=ON. GoogleTest is fetched on demand via FetchContent so the
# tests do not depend on a system-provided GTest package.

# Fetch GoogleTest only when actually building tests (keeps regular builds lean).
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
# On Windows / MSVC prevent gtest from overriding the parent project's settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_visualization_matrix
  test_visualization_matrix.cpp
)

target_link_libraries(test_visualization_matrix PRIVATE
  GTest::gtest
  GTest::gtest_main
  m  # math: log10f, cos, sqrt, ...
)

# Register the test with CTest. Using the explicit add_test() form (rather than
# gtest_discover_tests) keeps registration reliable without a build-time
# discovery step, matching the original project layout.
add_test(NAME test_visualization_matrix COMMAND test_visualization_matrix)
