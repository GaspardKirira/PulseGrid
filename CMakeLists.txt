cmake_minimum_required(VERSION 3.20)
project(PulseGrid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (UNIX)
  list(PREPEND CMAKE_PREFIX_PATH "/usr/local")
  list(APPEND CMAKE_IGNORE_PREFIX_PATH "$ENV{HOME}/.local")
endif()

option(VIX_LINK_STATIC "Static libstdc++/libgcc" OFF)
option(VIX_LINK_FULL_STATIC "Full static link (-static). Prefer musl." OFF)

set(VIX_LINK_STATIC OFF CACHE BOOL "Static libstdc++/libgcc" FORCE)
set(VIX_LINK_FULL_STATIC OFF CACHE BOOL "Full static link (-static). Prefer musl." FORCE)

set(VIX_ENABLE_DB ON CACHE BOOL "Enable Vix DB support" FORCE)
set(VIX_DB_USE_SQLITE ON CACHE BOOL "Enable SQLite support in Vix DB" FORCE)
set(VIX_DB_REQUIRE_SQLITE ON CACHE BOOL "Require SQLite support in Vix DB" FORCE)

# ------------------------------------------------------
# Core Vix runtime
# ------------------------------------------------------
find_package(Vix CONFIG REQUIRED)

# ------------------------------------------------------
# Local registry packages installed with: vix install
# ------------------------------------------------------
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
  elseif (VIX_LINK_STATIC)
    target_link_options(${tgt} PRIVATE -static-libstdc++ -static-libgcc)
    target_compile_definitions(${tgt} PRIVATE VIX_LINK_STATIC=1)
  endif()
endfunction()

# ------------------------------------------------------
# PulseGrid core library
# ------------------------------------------------------
add_library(pulsegrid_core
  src/pulsegrid/app/AppBootstrap.cpp
  src/pulsegrid/app/RouteRegistry.cpp
  src/pulsegrid/app/MiddlewareRegistry.cpp

  src/pulsegrid/presentation/http/MonitorController.cpp
  src/pulsegrid/presentation/http/StatusPageController.cpp
  src/pulsegrid/presentation/http/HealthController.cpp

  src/pulsegrid/presentation/ws/StatusWsGateway.cpp

  src/pulsegrid/application/services/MonitorService.cpp
  src/pulsegrid/application/services/CheckService.cpp
  src/pulsegrid/application/services/IncidentService.cpp
  src/pulsegrid/application/services/StatusService.cpp

  src/pulsegrid/infrastructure/db/DatabaseFactory.cpp
  src/pulsegrid/infrastructure/db/SqliteMonitorRepository.cpp
  src/pulsegrid/infrastructure/db/SqliteCheckResultRepository.cpp
  src/pulsegrid/infrastructure/db/SqliteIncidentRepository.cpp

  src/pulsegrid/infrastructure/events/WsEventBus.cpp

  src/pulsegrid/infrastructure/checker/HttpChecker.cpp
  src/pulsegrid/infrastructure/checker/CheckScheduler.cpp

  src/pulsegrid/support/Json.cpp
)

target_include_directories(pulsegrid_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(pulsegrid_core
  PUBLIC
    vix::vix
)

# Ajoute ici les libs Vix registry éventuelles si tu en utilises.
# Exemple :
# vix_link_optional_targets(pulsegrid_core
#   cnerium::app
#   cnerium::http
#   cnerium::json
# )

if (MSVC)
  target_compile_options(pulsegrid_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(pulsegrid_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

vix_apply_static_link_flags(pulsegrid_core)

# ------------------------------------------------------
# Main executable
# ------------------------------------------------------
add_executable(PulseGrid src/main.cpp)

target_link_libraries(PulseGrid
  PRIVATE
    pulsegrid_core
)

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

add_executable(PulseGrid_basic_test
  tests/test_basic.cpp
)

target_link_libraries(PulseGrid_basic_test
  PRIVATE
    pulsegrid_core
    vix::vix
)

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
