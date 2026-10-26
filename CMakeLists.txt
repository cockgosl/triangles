cmake_minimum_required(VERSION 3.16)
project(triangles LANGUAGES C CXX)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw glm)

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
    src/camera.cpp)

add_library(glad STATIC third_party/glad/src/gl.c)
add_library(triangles_lib STATIC ${SRCS})
target_include_directories(triangles_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(triangles_lib PUBLIC glm::glm)
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
        src/application.cpp
        src/triangle_renderer.cpp
        src/glfw_window.cpp
        src/shader_program.cpp
    )

    target_link_libraries(
        triangles.out
        PRIVATE
        triangles_lib
        glad
        glfw
        glm::glm
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
