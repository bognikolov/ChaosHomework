cmake_minimum_required(VERSION 3.15)
project(ChaosRayTracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG master
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
