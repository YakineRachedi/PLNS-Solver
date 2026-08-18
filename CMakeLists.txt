cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PLNS-SOLVER LANGUAGES CXX C)

set(USE_OPENMP FALSE)
if (USE_OPENMP)
	find_package(OpenMP)
endif()

add_library(PLNS-solver
    src/linalg/conjugate_gradient.cpp
)

target_include_directories(PLNS-solver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/matrix
)

target_include_directories(PLNS-solver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (USE_OPENMP AND OpenMP_CXX_FOUND)
	target_compile_options(PLNS-solver PRIVATE -DUSE_OPENMP)
	target_link_libraries(PLNS-solver PUBLIC OpenMP::OpenMP_CXX)
endif()

add_executable(test_cg
    src/bin/test_cg.cpp
)

target_link_libraries(test_cg PRIVATE PLNS-solver)