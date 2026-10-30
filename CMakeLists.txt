cmake_minimum_required(VERSION 3.20)
project(holdfast LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# core -- every game rule. No raylib, no window. This is what the tests test.
# =============================================================================
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp)
add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC include)
target_compile_options(core PRIVATE -Wall -Wextra -Wno-unused-parameter)

include(FetchContent)

# =============================================================================
# holdfast -- the playable game: core + raylib
# =============================================================================
option(HOLDFAST_BUILD_GAME "Build the raylib game (needs the X11/OpenGL dev packages in the dev container)" ON)
if(HOLDFAST_BUILD_GAME)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(raylib)
    target_compile_options(raylib PRIVATE -w)   # third-party warnings aren't ours to fix

    add_executable(holdfast src/game/main.cpp src/game/render.cpp)
    target_link_libraries(holdfast PRIVATE core raylib)
    target_compile_options(holdfast PRIVATE -Wall -Wextra -Wno-unused-parameter)
    target_compile_definitions(holdfast PRIVATE ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/levels")
endif()

# =============================================================================
# run_tests -- GoogleTest against core only (runs anywhere, no display)
# =============================================================================
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.cpp)
add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests PRIVATE core GTest::gtest_main)
target_compile_definitions(run_tests PRIVATE ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/levels")
