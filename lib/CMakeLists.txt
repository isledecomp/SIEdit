option(LIBWEAVER_BUILD_DOXYGEN "Build Doxygen documentation" OFF)

set(LIBWEAVER_HEADERS
  ${PROJECT_SOURCE_DIR}/include/libweaver/core.h
  ${PROJECT_SOURCE_DIR}/include/libweaver/file.h
  ${PROJECT_SOURCE_DIR}/include/libweaver/info.h
  ${PROJECT_SOURCE_DIR}/include/libweaver/interleaf.h
  ${PROJECT_SOURCE_DIR}/include/libweaver/object.h
  ${PROJECT_SOURCE_DIR}/include/libweaver/sitypes.h
  ${PROJECT_SOURCE_DIR}/include/libweaver/types.h
  ${PROJECT_SOURCE_DIR}/include/libweaver/util.h
)

set(LIBWEAVER_SOURCES
  core.cpp
  file.cpp
  interleaf.cpp
  object.cpp
  sitypes.cpp
)

add_library(libweaver SHARED
  ${LIBWEAVER_SOURCES}
  ${LIBWEAVER_HEADERS}
)
add_library(libweaver::libweaver ALIAS libweaver)
target_include_directories(libweaver PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/libweaver>")
target_include_directories(libweaver PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libweaver>")

target_compile_definitions(libweaver PRIVATE LIBWEAVER_LIBRARY)
if (NOT MSVC)
  target_compile_options(libweaver PRIVATE -Werror -Wall -Wextra -Wno-unused-parameter)
endif()
target_compile_definitions(libweaver PRIVATE $<$<BOOL:${WIN32}>:NOMINMAX>)
set_target_properties(libweaver PROPERTIES
  CXX_STANDARD 98
  CXX_STANDARD_REQUIRED ON
  PREFIX ""
  PUBLIC_HEADER "${LIBWEAVER_HEADERS}"
  IMPORT_PREFIX ""
)

if(LIBWEAVER_BUILD_DOXYGEN)
  find_package(Doxygen)
  set(DOXYGEN_PROJECT_NAME "libweaver")
  set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
  set(DOXYGEN_EXTRACT_ALL "YES")
  set(DOXYGEN_EXTRACT_PRIVATE "YES")
  doxygen_add_docs(docs ALL ${LIBWEAVER_SOURCES})
endif()

if(LIBWEAVER_INSTALL)
  install(TARGETS libweaver EXPORT libweaver-exports
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libweaver"
  )
endif()
