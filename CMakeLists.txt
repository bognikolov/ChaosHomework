cmake_minimum_required(VERSION 3.15)
project(ChaosRayTracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

include(FetchContent)
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG 24b5e7a8b27f42fa16b96fc70aade9106cf7102f
)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rapidjson)

add_executable(ChaosRayTracer src/main.cpp)

target_include_directories(ChaosRayTracer PRIVATE
    include
    ${rapidjson_SOURCE_DIR}/include
)
