cmake_minimum_required(VERSION 3.20)
project(PulseGrid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VIX_LINK_STATIC "Static libstdc++/libgcc" ON)
option(VIX_LINK_FULL_STATIC "Full static link (-static). Prefer musl." ON)

# ------------------------------------------------------
# Core Vix runtime
# ------------------------------------------------------
find_package(vix QUIET CONFIG)
if (NOT vix_FOUND)
  find_package(Vix CONFIG REQUIRED)
endif()

# ------------------------------------------------------
# Local registry packages installed with: vix install
# ------------------------------------------------------
# If you add packages from the Vix registry, they are wired
# through .vix/vix_deps.cmake. This file creates the imported
# targets for local project dependencies.
#
# Example:
#   vix add @cnerium/app
#   vix install
#
# Then uncomment the links you need below.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
  include("${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
endif()

# ------------------------------------------------------
# Helpers
# ------------------------------------------------------
function(vix_link_optional_targets tgt)
  foreach(dep IN LISTS ARGN)
    if (TARGET ${dep})
      target_link_libraries(${tgt} PRIVATE ${dep})
    endif()
  endforeach()
endfunction()

function(vix_apply_static_link_flags tgt)
  if (MSVC)
    return()
  endif()
  if (VIX_LINK_FULL_STATIC)
    target_link_options(${tgt} PRIVATE -static)
    target_compile_definitions(${tgt} PRIVATE VIX_LINK_FULL_STATIC=1)
  endif()
endfunction()

# ------------------------------------------------------
# Main executable
# ------------------------------------------------------
add_executable(PulseGrid src/main.cpp)
target_link_libraries(PulseGrid PRIVATE vix::vix)

# Add local registry libraries here.
# Keep them in this block so the build stays stable even if
# some packages are not installed yet.
#
# Example:
# vix_link_optional_targets(PulseGrid
#   cnerium::app
#   cnerium::http
#   cnerium::json
# )

if (MSVC)
  target_compile_options(PulseGrid PRIVATE /W4 /permissive-)
else()
  target_compile_options(PulseGrid PRIVATE -Wall -Wextra -Wpedantic)
endif()

vix_apply_static_link_flags(PulseGrid)

# ------------------------------------------------------
# Tests
# ------------------------------------------------------
include(CTest)
enable_testing()

add_executable(PulseGrid_basic_test tests/test_basic.cpp)
target_link_libraries(PulseGrid_basic_test PRIVATE vix::vix)

# Add the same local registry libraries to tests when needed.
# Example:
# vix_link_optional_targets(PulseGrid_basic_test
#   cnerium::app
#   cnerium::http
#   cnerium::json
# )

if (MSVC)
  target_compile_options(PulseGrid_basic_test PRIVATE /W4 /permissive-)
else()
  target_compile_options(PulseGrid_basic_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

vix_apply_static_link_flags(PulseGrid_basic_test)

add_test(NAME PulseGrid.basic COMMAND PulseGrid_basic_test)

# ------------------------------------------------------
# Convenience target
# ------------------------------------------------------
add_custom_target(run
  COMMAND $<TARGET_FILE:PulseGrid>
  DEPENDS PulseGrid
  USES_TERMINAL
)
