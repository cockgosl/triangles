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
    src/generator.cpp
    src/plane3.cpp
    src/line3.cpp
    src/point2.cpp
    src/intersection.cpp)

add_library(glad STATIC third_party/glad/src/gl.c)
add_library(triangles_lib STATIC ${SRCS})
target_include_directories(triangles_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(glad PUBLIC "${PROJECT_SOURCE_DIR}/third_party/glad/include")

set(SHADER_DIR "${PROJECT_SOURCE_DIR}/shaders")
set(SHADER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(MAKE_DIRECTORY "${SHADER_BUILD_DIR}")

configure_file(
    "${SHADER_DIR}/triangle.vert.glsl"
    "${SHADER_BUILD_DIR}/triangle.vert.glsl"
    COPYONLY
)

configure_file(
    "${SHADER_DIR}/triangle.frag.glsl"
    "${SHADER_BUILD_DIR}/triangle.frag.glsl"
    COPYONLY
)

option(BUILD_APP "Build the application" OFF)

if(BUILD_APP)
    add_executable(
        triangles.out
        src/main.cpp
        src/glfw_window.cpp
        src/shader_program.cpp
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
