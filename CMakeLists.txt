cmake_minimum_required(VERSION 3.0.0)
project(tracing)

# Set some compile flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_compile_options(-std=c++11 -Wall)

# OpenCV
find_package(OpenCV REQUIRED)

add_executable(tracing
	src/tracing.cpp
	src/EdgeProcessor.cpp
	src/EdgeIdMap.cpp
	src/Edges.cpp
	src/Visualizer.cpp
	src/Visualizer2.cpp
	src/ConfigParser.cpp
	src/EdgesConnection.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Linking
target_link_libraries(tracing ${OpenCV_LIBS})
