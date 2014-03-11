cmake_minimum_required(VERSION 2.8)

project(qvtf)

# ---- dependencies -----------------------------------------------------------
find_package(Qt4 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(VTFLIB REQUIRED VTFLib)

# ---- compiler flags ---------------------------------------------------------
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -Werror -Wno-write-strings -O3")
endif()

include_directories(${VTFLIB_INCLUDE_DIRS})

if(NOT(${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
	add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif()

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

# ---- build library ----------------------------------------------------------
qt4_wrap_cpp(vtf_MOC LoadValveTextureFile.h)
add_library(qvtf SHARED LoadValveTextureFile.cpp LoadValveTextureFile.h ${vtf_MOC})
target_link_libraries(qvtf ${QT_LIBRARIES} ${VTFLIB_LIBRARIES})

# ---- install target ---------------------------------------------------------
install(TARGETS qvtf DESTINATION "${QT_IMAGEFORMATS_PLUGINS_DIR}")

# ---- uninstall target -------------------------------------------------------
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
