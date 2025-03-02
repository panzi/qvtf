cmake_minimum_required(VERSION 3.16)

project(qvtf)

# ---- dependencies -----------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Gui)

find_package(ECM 6.11.0  NO_MODULE)
if (${ECM_FOUND})
	set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
	include(KDECompilerSettings)
endif()

find_package(PkgConfig REQUIRED)
find_program(XDG_MIME_EXECUTABLE xdg-mime) # optional, for MIME type installation

pkg_check_modules(VTFLIB REQUIRED VTFLib)

# ---- compiler flags ---------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -Werror -Wno-write-strings -Wno-suggest-override -O3")
endif()

include_directories(${VTFLIB_INCLUDE_DIRS})

if(NOT(${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
	add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif()

# ---- build library ----------------------------------------------------------
qt_add_plugin(qvtf PLUGIN_TYPE imageformats)
target_sources(qvtf PRIVATE ValveTextureFile.cpp)
target_link_libraries(qvtf Qt6::Gui ${VTFLIB_LIBRARIES})

# ---- install target ---------------------------------------------------------
install(TARGETS qvtf DESTINATION ${QT6_INSTALL_PLUGINS}/imageformats)

if(NOT(${XDG_MIME_EXECUTABLE} STREQUAL "XDG_MIME_EXECUTABLE-NOTFOUND"))
	install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor \"${PROJECT_SOURCE_DIR}/vtf.xml\")")
endif()
