add_library(libweaver SHARED
  # Data types
  data/data.cpp
  data/data.h
  data/generic.cpp
  data/generic.h
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

target_compile_definitions(libweaver PRIVATE LIBWEAVER_LIBRARY)
set_target_properties(libweaver PROPERTIES
  CXX_STANDARD 98
  CXX_STANDARD_REQUIRED ON
)
