cmake_minimum_required(VERSION 3.16)
project(triangles LANGUAGES C CXX)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRCS
    src/vector3.cpp
    src/triangle.cpp
    src/generator.cpp)

add_library(glad STATIC third_party/glad/src/gl.c)
add_library(triangles_lib STATIC ${SRCS})
target_include_directories(triangles_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(glad PUBLIC "${PROJECT_SOURCE_DIR}/third_party/glad/include")

option(BUILD_APP "Build the application" OFF)

if(BUILD_APP)
    add_executable(
        triangles.out
        src/main.cpp
        src/graphics.cpp
    )

    target_link_libraries(
        triangles.out
        PRIVATE
        triangles_lib
        glad
        glfw
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
