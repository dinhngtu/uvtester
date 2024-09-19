cmake_minimum_required(VERSION 3.13)

project(uvtester CXX)

find_package(asmjit CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(uvtester)
target_sources(uvtester
    PRIVATE
        uvtester.cpp
        uvfault.cpp
        uvfault.hpp
)
target_link_libraries(uvtester PRIVATE asmjit::asmjit)
