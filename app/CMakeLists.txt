find_package(Qt6)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

find_package(FFMPEG 3.0 REQUIRED
  COMPONENTS
  avutil
  avcodec
  avformat
  avfilter
  swscale
  swresample
)

set(PROJECT_SOURCES
  siview/chunkmodel.cpp
  siview/chunkmodel.h
  siview/infopanel.cpp
  siview/infopanel.h
  siview/siview.cpp
  siview/siview.h

  viewer/mediapanel.cpp
  viewer/mediapanel.h

  main.cpp
  mainwindow.cpp
  mainwindow.h
  model.cpp
  model.h
  objectmodel.cpp
  objectmodel.h
  panel.cpp
  panel.h
  vector3edit.cpp
  vector3edit.h

  res/res.rc
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

target_link_libraries(si-edit PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Multimedia
  FFMPEG::avutil
  FFMPEG::avcodec
  FFMPEG::avformat
  FFMPEG::avfilter
  FFMPEG::swscale
  FFMPEG::swresample
  libweaver)
target_include_directories(si-edit PRIVATE "${CMAKE_SOURCE_DIR}/lib" ${FFMPEG_INCLUDE_DIRS})
if (NOT MSVC)
  target_compile_options(si-edit PRIVATE -Werror)
endif()

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
