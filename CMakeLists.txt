cmake_minimum_required(VERSION 2.8)  # we could bump to >3.4
set(CMAKE_COLOR_MAKEFILE ON)

# Define project name
project(geotiff_test)

# Looking for GDAL;: https://cmake.org/cmake/help/v3.0/module/FindGDAL.html
find_package(GDAL 2.2 REQUIRED)
message(STATUS "GDAL library status:")
#message(STATUS "    version: ${GDAL_version}") # FindGDAL.cmake does not provide VERSION info for cmake < 3.13
message(STATUS "    libraries: ${GDAL_LIBRARY}")
message(STATUS "    include path: ${GDAL_INCLUDE_DIR}")

# Greedy include of all headers
file(GLOB PROJECT_HEADERS include/*.h include/*.hpp)
include_directories(BEFORE ../include
                    include
                    ${GDAL_INCLUDE_DIR})
# Retrieve git commit information, forward it to compilation time
exec_program(
    "git"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS "describe --abbrev=4 --dirty --always --tags"
    OUTPUT_VARIABLE GIT_INFO )
# fwd git commit info via GIT_COMMIT variable
add_definitions( -DGIT_COMMIT="${GIT_INFO}" )

############################ GDAL_TEST ####################
add_executable (geotiff_test  	src/geotiff_test.cpp
                          		src/geotiff.cpp
                          		${PROJECT_HEADERS})

target_compile_options(geotiff_test PUBLIC -std=c++11 -pthread)
target_link_libraries(geotiff_test ${GDAL_LIBRARY})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
