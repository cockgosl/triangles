cmake_minimum_required(VERSION 3.16)
project(triangles LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRCS
    src/vector3.cpp
    src/triangle.cpp)

add_library(triangles_lib STATIC ${SRCS})
target_include_directories(triangles_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_executable(triangles.out src/main.cpp)
target_link_libraries(triangles.out PRIVATE triangles_lib)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
