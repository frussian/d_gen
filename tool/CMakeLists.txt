cmake_minimum_required(VERSION 3.15)

project(d_gen_tool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(d_gen REQUIRED)

set(sources main.cpp)

add_executable(d_gen_tool)
target_sources(d_gen_tool PRIVATE ${sources})
target_link_libraries(d_gen_tool PRIVATE d_gen::d_gen)
