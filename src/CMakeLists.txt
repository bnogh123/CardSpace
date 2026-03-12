# ==============================================================================
# cardspace_core — the compiled library that apps and wrappers link against
# ==============================================================================

add_library(cardspace_core
  # HTTP / fetch layer
  scryfall_client.cpp       # libcurl wrapper: downloads JSON from Scryfall API

  # ETL layer
  etl_pipeline.cpp          # orchestrates extract → transform → load
  card_parser.cpp           # deserialises raw Scryfall JSON into Card structs
  transformer.cpp           # cleans / normalises fields (mana cost, legalities…)
  loader.cpp                # writes transformed records to the data warehouse
  CardType.cpp              # encodes / de-encodes supertype and type typeline

  # Data warehouse layer
  warehouse.cpp             # schema management and bulk insert helpers

  # Utilities
  utils.cpp                 # logging, string helpers, file I/O
)

# Public header directory — consumers of this lib get these includes automatically
target_include_directories(cardspace_core
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}   # for internal .hpp files living next to .cpp
)

target_link_libraries(cardspace_core
  PUBLIC
    nlohmann_json::nlohmann_json  # JSON parsing exposed in public API
  PRIVATE
    CURL::libcurl                 # HTTP — implementation detail only
    fmt::fmt                      # formatting / logging — implementation detail
)

# Require C++17 for std::filesystem, structured bindings, etc.
target_compile_features(cardspace_core PUBLIC cxx_std_17)
