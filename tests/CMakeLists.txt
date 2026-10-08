include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

foreach(object IN ITEMS vector3 triangle)
    add_executable(${object}_tests ${object}_tests.cpp)
    set_target_properties(${object}_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

    target_link_libraries(${object}_tests PRIVATE triangles_lib GTest::gtest_main)

    target_compile_options(${object}_tests PRIVATE -Wall -Wextra -Wpedantic)
    gtest_discover_tests(${object}_tests PROPERTIES TIMEOUT 10 LABELS "${object}")
endforeach()
