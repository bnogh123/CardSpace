# ==============================================================================
# Tests — uses Catch2 (fetched automatically)
# ==============================================================================
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.4)
FetchContent_MakeAvailable(Catch2)

# Helper to reduce boilerplate when adding test executables
# Usage: cardspace_add_test(test_name source1.cpp source2.cpp ...)
function(cardspace_add_test test_name)
  add_executable(${test_name} ${ARGN})
  target_link_libraries(${test_name}
    PRIVATE
      cardspace_core
      Catch2::Catch2WithMain)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# --- Individual test suites ---------------------------------------------------
cardspace_add_test(test_card_parser    test_card_parser.cpp)
cardspace_add_test(test_transformer    test_transformer.cpp)
cardspace_add_test(test_etl_pipeline   test_etl_pipeline.cpp)
cardspace_add_test(test_warehouse      test_warehouse.cpp)
