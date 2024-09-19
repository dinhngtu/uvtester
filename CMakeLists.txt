cmake_minimum_required(VERSION 3.13)

project(uvtester CXX)

find_package(asmjit REQUIRED)
find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)

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
target_compile_options(uvtester
    PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Werror=shadow -Werror=return-type -fno-strict-aliasing>
)
target_link_libraries(uvtester PRIVATE asmjit fmt)
