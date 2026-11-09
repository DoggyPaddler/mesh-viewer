cmake_minimum_required(VERSION 3.5)
project(mesh-viewer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-deprecated-declarations -Wno-unused-function")
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm" PROPERTIES COMPILE_FLAGS "-x objective-c++")

find_library(COCOA_LIBRARY Cocoa)
find_library(CARBON_LIBRARY Carbon)

add_subdirectory(lib/nanogui)
include_directories(lib/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})

set(MM_EXTRA_SOURCE "")
set(FILE_DIALOG_MM "${CMAKE_CURRENT_SOURCE_DIR}/src/fileDialog.mm")
if(EXISTS "${FILE_DIALOG_MM}")
	list(APPEND MM_EXTRA_SOURCE "${FILE_DIALOG_MM}")
endif()

include_directories(src)
add_definitions(${NANOGUI_EXTRA_DEFS})
add_subdirectory(lib/soil soil)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(APPEND SOURCES ${MM_EXTRA_SOURCE})
#list(REMOVE_ITEM SOURCES "lib/nanogui/src/example*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC lib/soil/inc)
find_path(OPENEXR_INCLUDE_DIR OpenEXR/ImfRgbaFile.h)
find_library(OPENEXR_LIBRARY NAMES IlmImf OpenEXR)
if(NOT OPENEXR_INCLUDE_DIR OR NOT OPENEXR_LIBRARY)
	message(FATAL_ERROR "OpenEXR headers and library are required to load environment maps")
endif()
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${OPENEXR_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY} ${CARBON_LIBRARY} nanogui ${NANOGUI_EXTRA_LIBS} soil ${OPENEXR_LIBRARY})
