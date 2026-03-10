# ==============================================================================
# Executables
# ==============================================================================

# --- Main ETL runner ----------------------------------------------------------
# Calls the full pipeline: fetch → transform → load into warehouse
add_executable(cardspace_etl main.cpp)

target_link_libraries(cardspace_etl PRIVATE cardspace_core)

# --- Quick analysis CLI -------------------------------------------------------
# Lightweight tool for ad-hoc queries against the local warehouse
# (useful for sanity-checking data before handing off to Python/Java)
add_executable(cardspace_query query_main.cpp)

target_link_libraries(cardspace_query PRIVATE cardspace_core)

# Both executables land in PROJECT_SOURCE_DIR/bin (set in root CMakeLists)
