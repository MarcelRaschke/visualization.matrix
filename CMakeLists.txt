cmake_minimum_required(VERSION 3.20)
project(visualization.matrix VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)

find_package(Kodi REQUIRED)
find_package(glm REQUIRED)
add_subdirectory(lib/kissfft)

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

include_directories(
  ${GLM_INCLUDE_DIR}
  ${KODI_INCLUDE_DIR}/..
  ${PROJECT_SOURCE_DIR}/lib
)

if(CORE_SYSTEM_NAME STREQUAL osx OR
   CORE_SYSTEM_NAME STREQUAL ios OR
   CORE_SYSTEM_NAME STREQUAL darwin_embedded)
  list(APPEND DEPLIBS "-framework CoreVideo")
endif()

set(MATRIX_SOURCES
  src/main.cpp
)

set(MATRIX_HEADERS
  src/main.h
  src/stb_image.h
)

list(APPEND DEPLIBS kissfft)
build_addon(visualization.matrix MATRIX DEPLIBS)

if(TARGET visualization.matrix)
  target_compile_options(visualization.matrix PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
  )

  if(ENABLE_COVERAGE)
    target_compile_options(visualization.matrix PRIVATE --coverage)
    target_link_options(visualization.matrix PRIVATE --coverage)
  endif()

  if(ENABLE_CLANG_TIDY AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_target_properties(visualization.matrix PROPERTIES
      CXX_CLANG_TIDY "clang-tidy;-checks=*;-warnings-as-errors=*"
    )
  endif()
endif()

include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Team Kodi")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Matrix themed visualization addon for Kodi")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md)

# Enable testing if requested
if(BUILD_TESTING)
  enable_testing()
  # Add tests here when available
endif()
