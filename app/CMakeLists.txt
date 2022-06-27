find_package(Qt5)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
  panels/mxch.cpp
  panels/mxch.h
  panels/mxhd.cpp
  panels/mxhd.h
  panels/mxob.cpp
  panels/mxob.h
  panels/mxof.cpp
  panels/mxof.h
  panels/panel.cpp
  panels/panel.h
  panels/riff.cpp
  panels/riff.h

  chunkmodel.cpp
  chunkmodel.h
  main.cpp
  mainwindow.cpp
  mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(si-edit
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
  )
else()
  add_executable(si-edit
    ${PROJECT_SOURCES}
  )
endif()

target_link_libraries(si-edit PRIVATE Qt${QT_VERSION_MAJOR}::Widgets libweaver)
target_include_directories(si-edit PRIVATE "${CMAKE_SOURCE_DIR}/lib")

set_target_properties(si-edit PROPERTIES
  MACOSX_BUNDLE_GUI_IDENTIFIER com.mattkc.SIEdit
  MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  AUTOUIC ON
  AUTOMOC ON
  AUTORCC ON
)

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(si-edit)
endif()
