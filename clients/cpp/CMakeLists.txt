cmake_minimum_required(VERSION 3.5)
project(aicup2019)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-std=c++17 -D_LINUX -O3 -g -Wall")

file(GLOB HEADERS "*.hpp" "model/*.hpp" "csimplesocket/*.h")
SET_SOURCE_FILES_PROPERTIES(${HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
file(GLOB SRC "*.cpp" "model/*.cpp" "csimplesocket/*.cpp")
add_executable(aicup2019 ${HEADERS} ${SRC} arena/Simulation.hpp arena/Simulation.cpp utils/Geometry.h Consts.h model/Vec2.hpp arena/MinMax.cpp arena/MinMax.h utils/Segment.cpp utils/Segment.h arena/Target.cpp arena/Target.h)
TARGET_LINK_LIBRARIES(aicup2019 ${PROJECT_LIBS})