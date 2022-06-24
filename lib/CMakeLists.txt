option(LIBWEAVER_BUILD_DOXYGEN "Build Doxygen documentation" OFF)

set(LIBWEAVER_SOURCES
  # Data types
  data/data.cpp
  data/data.h
  data/generic.cpp
  data/generic.h
  data/mxhd.cpp
  data/mxhd.h
  data/mxst.cpp
  data/mxst.h
  data/riff.cpp
  data/riff.h

  # Chunk
  chunk.cpp
  chunk.h

  # Common
  libweaver_global.h
  types.h
)

add_library(libweaver SHARED
  ${LIBWEAVER_SOURCES}
)

target_compile_definitions(libweaver PRIVATE LIBWEAVER_LIBRARY)
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
