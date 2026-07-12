cmake_minimum_required(VERSION 3.24)

project(reference_software_renderer LANGUAGES C CXX)

enable_testing()

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

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../vendor/raylib" "${CMAKE_CURRENT_BINARY_DIR}/raylib")
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

add_executable(reference_software_renderer
  main.cpp
  allocators/allocator.h
  allocators/allocator.cpp
  allocators/aligned_allocator.h
  allocators/aligned_allocator.cpp
  allocators/arena_allocator.h
  allocators/arena_allocator.cpp
  app/application.h
  app/application.cpp
  app/cube_field.h
  app/cube_field.cpp
  app/cube_view.h
  app/cube_view.cpp
  sdk/orbit_camera.h
  sdk/orbit_camera.cpp
  sdk/rendering.h
  sdk/rendering.cpp
  sdk/runtime.h
  sdk/runtime.cpp
)
target_include_directories(reference_software_renderer PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(reference_software_renderer PRIVATE raylib)
if(MSVC)
  target_compile_options(reference_software_renderer PRIVATE /W4)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(reference_software_renderer PRIVATE -Wno-missing-field-initializers)
  endif()
endif()

add_executable(reference_core_tests
  tests_core.cpp
  allocators/allocator.h
  allocators/allocator.cpp
  allocators/aligned_allocator.h
  allocators/aligned_allocator.cpp
  allocators/arena_allocator.h
  allocators/arena_allocator.cpp
  app/cube_field.h
  app/cube_field.cpp
  app/cube_view.h
  app/cube_view.cpp
)
target_include_directories(reference_core_tests PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(reference_core_tests PRIVATE raylib)
if(MSVC)
  target_compile_options(reference_core_tests PRIVATE /W4)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(reference_core_tests PRIVATE -Wno-missing-field-initializers)
  endif()
endif()
add_test(NAME reference_core COMMAND reference_core_tests)

add_custom_target(reference_software_renderer_assets
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
          "${CMAKE_CURRENT_LIST_DIR}/../assets"
          "$<TARGET_FILE_DIR:reference_software_renderer>/assets"
)
add_dependencies(reference_software_renderer reference_software_renderer_assets)
