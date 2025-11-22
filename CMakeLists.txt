cmake_minimum_required(VERSION 3.21)

project(libweaver VERSION 1.0 LANGUAGES CXX)

option(LIBWEAVER_INSTALL "Enable libweaver install targets" "${PROJECT_IS_TOP_LEVEL}")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(lib)
add_subdirectory(app)

if(LIBWEAVER_INSTALL)
  export(TARGETS libweaver NAMESPACE libweaver:: FILE "libweaver-targets.cmake")
  if(MSVC)
    set(libweaver_install_cmakedir "cmake")
  else()
    set(libweaver_install_cmakedir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
  endif()
  configure_package_config_file(cmake/Configlibweaver.cmake libweaverConfig.cmake
    INSTALL_DESTINATION "${libweaver_install_cmakedir}"
  )
  write_basic_package_version_file(libweaverConfigVersion.cmake
    VERSION "${libweaver_VERSION}"
    COMPATIBILITY SameMajorVersion
  )
  install(
    FILES
      "${CMAKE_CURRENT_BINARY_DIR}/libweaverConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/libweaverConfigVersion.cmake"
    DESTINATION "${libweaver_install_cmakedir}"
  )
  install(EXPORT libweaver-exports
    DESTINATION "${libweaver_install_cmakedir}"
    FILE "libweaver-targets.cmake"
    NAMESPACE "libweaver::"
  )
endif()
