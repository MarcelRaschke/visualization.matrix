cmake_minimum_required(VERSION 3.20)
project(visualization.matrix VERSION 1.0.0 LANGUAGES CXX)

# Project metadata
set(PROJECT_DESCRIPTION "Matrix themed visualization addon for Kodi")
set(PROJECT_AUTHOR "Team Kodi")
set(PROJECT_LICENSE "GPL-2.0-or-later")

# Set C++ standard (C++17 for Kodi Matrix compatibility)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

# Options
option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck" OFF)
option(ENABLE_TRACY "Enable Tracy profiler instrumentation (requires tracy headers when enabled)" OFF)
option(TRACY_ON_DEMAND "Only collect Tracy zones when a profiler is attached" ON)

# Fetch kissfft dependency
include(FetchContent)
FetchContent_Declare(
  kissfft
  GIT_REPOSITORY https://github.com/mborgerding/kissfft.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(kissfft)

# Find required packages
find_package(Kodi REQUIRED)
find_package(glm REQUIRED)

# Platform-specific OpenGL handling
if(NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

# Include directories
include_directories(
  ${GLM_INCLUDE_DIR}
  ${KODI_INCLUDE_DIR}/..
  ${PROJECT_SOURCE_DIR}/lib
  ${kissfft_SOURCE_DIR}
)

# Platform-specific libraries
if(CORE_SYSTEM_NAME STREQUAL osx OR
   CORE_SYSTEM_NAME STREQUAL ios OR
   CORE_SYSTEM_NAME STREQUAL darwin_embedded)
  list(APPEND DEPLIBS "-framework CoreVideo")
endif()

# Source files
set(MATRIX_SOURCES
  src/main.cpp
  ${kissfft_SOURCE_DIR}/kiss_fft.c
)

set(MATRIX_HEADERS
  src/main.h
  src/math_utils.h
  src/tracyprofiler.h
  src/stb_image.h
  ${kissfft_SOURCE_DIR}/kiss_fft.h
  ${kissfft_SOURCE_DIR}/kiss_fft_log.h
  ${kissfft_SOURCE_DIR}/_kiss_fft_guts.h
)

# Create kissfft library target
add_library(kissfft STATIC
  ${kissfft_SOURCE_DIR}/kiss_fft.c
  ${kissfft_SOURCE_DIR}/kiss_fft.h
  ${kissfft_SOURCE_DIR}/kiss_fft_log.h
  ${kissfft_SOURCE_DIR}/_kiss_fft_guts.h
)
target_include_directories(kissfft PUBLIC ${kissfft_SOURCE_DIR})

# Main visualization addon
add_library(visualization.matrix MODULE
  ${MATRIX_SOURCES}
  ${MATRIX_HEADERS}
)

# Link libraries
target_link_libraries(visualization.matrix PRIVATE
  kissfft
  ${DEPLIBS}
  ${KODI_LIBRARIES}
)

# Set target properties
target_compile_options(visualization.matrix PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Install target
install(TARGETS visualization.matrix
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# CPack configuration
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR ${PROJECT_AUTHOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md)

# Code coverage (gcov/lcov)
#
# Build with: cmake -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON ..
# Run tests, then: lcov --capture --directory . --output-file coverage.info
#                  genhtml coverage.info --output-directory coverage-html
#
# Configured before the test subdirectory so the test target inherits the
# coverage compile/link flags too.
if(ENABLE_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "ENABLE_COVERAGE requires GCC or Clang")
  endif()
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# Static analysis integration
#
# Clang-Tidy: enabled with -DENABLE_CLANG_TIDY=ON (Clang toolchain recommended).
#   Runs as part of the build; failures (treated as errors) break the build.
# Cppcheck:   enabled with -DENABLE_CPPCHECK=ON. Runs as a build hook on the
#   main target's sources; non-zero exit fails the build.
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_BIN NAMES clang-tidy clang-tidy-14 clang-tidy-15 clang-tidy-16)
  if(CLANG_TIDY_BIN)
    set(CMAKE_CXX_CLANG_TIDY
      "${CLANG_TIDY_BIN};-checks=*,-modernize-use-trailing-return-type;-warnings-as-errors=*;-quiet")
    message(STATUS "clang-tidy enabled: ${CLANG_TIDY_BIN}")
  else()
    message(WARNING "ENABLE_CLANG_TIDY=ON but clang-tidy not found; skipping.")
  endif()
endif()

if(ENABLE_CPPCHECK)
  find_program(CPPCHECK_BIN NAMES cppcheck)
  if(CPPCHECK_BIN)
    set(CMAKE_CXX_CPPCHECK
      "${CPPCHECK_BIN};--enable=warning,style;--inconclusive;--std=c++17;--suppress=missingIncludeSystem;--error-exitcode=1")
    message(STATUS "cppcheck enabled: ${CPPCHECK_BIN}")
  else()
    message(WARNING "ENABLE_CPPCHECK=ON but cppcheck not found; skipping.")
  endif()
endif()

# Tracy profiler integration (optional).
#
# When ENABLE_TRACY is ON and the tracy headers are available (TracyClient.cpp
# is expected to be compiled by the consuming project or via FetchContent), the
# addon is instrumented with zone scopes. When the headers are absent, the
# include is skipped and profiling is a no-op, so enabling the option is safe
# even without tracy installed.
if(ENABLE_TRACY)
  find_path(TRACY_INCLUDE_DIR NAMES tracy/Tracy.hpp)
  if(TRACY_INCLUDE_DIR)
    target_compile_definitions(visualization.matrix PRIVATE TRACY_ENABLE)
    if(TRACY_ON_DEMAND)
      target_compile_definitions(visualization.matrix PRIVATE TRACY_ON_DEMAND)
    endif()
    target_include_directories(visualization.matrix PRIVATE ${TRACY_INCLUDE_DIR})
    message(STATUS "Tracy profiler enabled (include: ${TRACY_INCLUDE_DIR})")
  else()
    message(STATUS "ENABLE_TRACY=ON but tracy headers not found; instrumentation disabled.")
  endif()
endif()

# Enable testing if requested. Done after coverage/analysis setup so the test
# target inherits the same compile/link flags (e.g. --coverage).
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()
