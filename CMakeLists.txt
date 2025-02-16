cmake_minimum_required(VERSION 3.20)
project(ug2extract)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE UG2E_SRC_FILES src/*.cpp)
file(GLOB_RECURSE UG2E_HEADER_FILES include/*.h)

add_executable(ug2e ${UG2E_SRC_FILES} ${UG2E_HEADER_FILES})
target_include_directories(ug2e PRIVATE include)