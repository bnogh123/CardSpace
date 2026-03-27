cmake_minimum_required(VERSION 3.14)

project(
  CardSpace
  VERSION 0.1.0
  DESCRIPTION "Scryfall ETL pipeline and data warehouse backbone"
  LANGUAGES CXX)

# ==============================================================================
# Project-wide settings (only when this is the top-level project)
# ==============================================================================
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)               # enforce -std=c++17, not -std=g++17

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Output binaries to a predictable location
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

  # CTest support
  include(CTest)

  # Doxygen docs (optional)
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, skipping docs")
  endif()

endif()

# ==============================================================================
# Dependencies
# ==============================================================================
include(FetchContent)

# --- libcurl (system package preferred; fallback message) ---------------------
find_package(CURL REQUIRED)
# Links via CURL::libcurl

# --- nlohmann/json (you already have the header; this just makes it a target) -
# If include/nlohmann/json.hpp is present we create an INTERFACE target for it.
# Alternatively, FetchContent can pull it automatically.
if(EXISTS "${PROJECT_SOURCE_DIR}/include/nlohmann/json.hpp")
  add_library(nlohmann_json INTERFACE)
  target_include_directories(nlohmann_json INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
  add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
  message(STATUS "Using bundled nlohmann/json")
else()
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3)
  FetchContent_MakeAvailable(nlohmann_json)
  message(STATUS "Fetching nlohmann/json from GitHub")
endif()

# --- fmtlib -------------------------------------------------------------------
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        10.2.1)           # updated from 5.3.0
FetchContent_MakeAvailable(fmtlib)
# Links via fmt::fmt

# --- SQLite -------------------------------------------------------------------
find_package(SQLite3 REQUIRED)
# Links via SQLite3::SQLite3


# ==============================================================================
# Project subdirectories
# ==============================================================================

# Core library (ETL logic, HTTP client, data models)
add_subdirectory(src)

# CLI / runner executables
add_subdirectory(apps)

# Tests (only when building as top-level project or via override flag)
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR CARDSPACE_BUILD_TESTING)
   AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
