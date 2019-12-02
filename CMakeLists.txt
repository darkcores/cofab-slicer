cmake_minimum_required(VERSION 3.9.0)
project(slicer LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

SET(GCC_FLAGS "-march=native -mtune=native -O3 -msse -msse2 -msse3 -mssse3 -std=c++14 -Wall -Wextra -pedantic")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_FLAGS}")

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt53DCore REQUIRED)
find_package(Qt53DExtras REQUIRED)
find_package(assimp REQUIRED)
if(ASSIMP_FOUND)
    include_directories(${ASSIMP_INCLUDE_DIR})
endif() 

include(FindOpenMP)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

include_directories("./src")
include_directories("./3rdparty")

set(SOURCES
  3rdparty/polyclipping/clipper.cpp
  src/sliceviewer.cpp
  src/slicewidget.cpp
  src/mainwindow.cpp
  src/model3d.cpp
  src/sliceprocessor.cpp
  src/orbitcameracontroller.cpp
  src/sliceroptions.cpp
  src/stlviewer.cpp
)

add_executable(slicer main.cpp ${SOURCES})
target_link_libraries(slicer ${ASSIMP_LIBRARIES})
target_link_libraries(slicer Qt5::Core)
target_link_libraries(slicer Qt5::Widgets)
target_link_libraries(slicer Qt5::3DCore)
target_link_libraries(slicer Qt5::3DExtras)
# qt5_use_modules(slicer Core Gui Widgets Network 3DCore 3DExtras)

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
include(CheckIPOSupported) 
check_ipo_supported(RESULT result OUTPUT output)
if(result)
  set_property(TARGET slicer PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${output}")
endif()
