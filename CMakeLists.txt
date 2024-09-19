cmake_minimum_required(VERSION 3.13)

project(uvtester CXX)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

find_package(asmjit CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

add_executable(uvtester)
target_sources(uvtester
    PRIVATE
        uvtester.cpp
        uvfault.cpp
        uvfault.hpp
)
target_link_libraries(uvtester PRIVATE asmjit::asmjit)
