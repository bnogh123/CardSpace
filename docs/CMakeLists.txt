# ==============================================================================
# Doxygen documentation — only reachable when Doxygen is found (root checks)
# ==============================================================================
set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")

doxygen_add_docs(
  cardspace_docs
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
  COMMENT "Generating CardSpace API documentation")
