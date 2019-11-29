cmake_minimum_required(VERSION 3.5)
project(aicup2019)

file(GLOB HEADERS "*.hpp" "model/*.hpp" "csimplesocket/*.h")
SET_SOURCE_FILES_PROPERTIES(${HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
file(GLOB SRC "*.cpp" "model/*.cpp" "csimplesocket/*.cpp")
add_executable(aicup2019 ${HEADERS} ${SRC} arena/Arena.hpp arena/Arena.cpp)
TARGET_LINK_LIBRARIES(aicup2019 ${PROJECT_LIBS})