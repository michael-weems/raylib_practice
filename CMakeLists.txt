cmake_minimum_required(VERSION 3.24)

project(software_renderer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  foreach(lang C CXX)
    foreach(config DEBUG RELWITHDEBINFO)
      string(REPLACE "/Zi" "/Z7" CMAKE_${lang}_FLAGS_${config} "${CMAKE_${lang}_FLAGS_${config}}")
    endforeach()
  endforeach()
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RAUDIO OFF CACHE BOOL "" FORCE)
set(PLATFORM "RGFW" CACHE STRING "" FORCE)
set(OPENGL_VERSION "Software" CACHE STRING "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/vendor/raylib" "${CMAKE_CURRENT_BINARY_DIR}/raylib")
if(WIN32)
  target_compile_definitions(raylib PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(raylib PRIVATE
    -Wno-initializer-overrides
    -Wno-static-in-inline
  )
endif()

# TODO: Re-test RLSW_USE_SIMD_INTRINSICS after the vendored MSVC SSE2 path
# fixes its _mm_loadu_si32 initialization error in external/rlsw.h.

add_executable(software_renderer
  src/main.cpp
  src/sdk/orbit_camera.h
  src/sdk/orbit_camera.cpp
  src/sdk/runtime.h
  src/sdk/runtime.cpp
)
target_include_directories(software_renderer PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(software_renderer PRIVATE raylib)
if(MSVC)
  target_compile_options(software_renderer PRIVATE /W4)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(software_renderer PRIVATE -Wno-missing-field-initializers)
  endif()
endif()

#[[
enable_testing()
add_executable(software_renderer_tests
  tests/core_tests.cpp
)
target_include_directories(software_renderer_tests PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
target_link_libraries(software_renderer_tests PRIVATE raylib)
if(MSVC)
  target_compile_options(software_renderer_tests PRIVATE /W4)
endif()
add_test(NAME software_renderer_core COMMAND software_renderer_tests)

]]

add_custom_target(software_renderer_assets
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
          "${CMAKE_CURRENT_LIST_DIR}/assets"
          "$<TARGET_FILE_DIR:software_renderer>/assets"
)
add_dependencies(software_renderer software_renderer_assets)
