option(LIBWEAVER_BUILD_DOXYGEN "Build Doxygen documentation" OFF)

set(LIBWEAVER_SOURCES
  common.h
  core.cpp
  core.h
  interleaf.cpp
  interleaf.h
  object.cpp
  object.h
  sitypes.cpp
  sitypes.h
  types.h
  util.h
)

add_library(libweaver SHARED
  ${LIBWEAVER_SOURCES}
)

target_compile_definitions(libweaver PRIVATE LIBWEAVER_LIBRARY)
if (NOT MSVC)
  target_compile_options(libweaver PRIVATE -Werror -Wall -Wextra -Wno-unused-parameter)
endif()
set_target_properties(libweaver PROPERTIES
  CXX_STANDARD 98
  CXX_STANDARD_REQUIRED ON
)

if(LIBWEAVER_BUILD_DOXYGEN)
  find_package(Doxygen)
  set(DOXYGEN_PROJECT_NAME "libweaver")
  set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
  set(DOXYGEN_EXTRACT_ALL "YES")
  set(DOXYGEN_EXTRACT_PRIVATE "YES")
  doxygen_add_docs(docs ALL ${LIBWEAVER_SOURCES})
endif()
