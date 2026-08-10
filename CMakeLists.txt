cmake_minimum_required(VERSION 3.15)
project(ChaosRayTracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

include(FetchContent)
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    # Pinned to a specific commit (master as of 2025-02-05) instead of a moving
    # branch, so the build is reproducible and doesn't silently pick up upstream
    # changes between builds.
    GIT_TAG 24b5e7a8b27f42fa16b96fc70aade9106cf7102f
)
FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif()

add_executable(ChaosRayTracer src/main.cpp)

target_include_directories(ChaosRayTracer PRIVATE
    include
    ${rapidjson_SOURCE_DIR}/include
)
