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


###############################################################################
# Library
###############################################################################

add_library(PLNS-solver
    src/linalg/conjugate_gradient.cpp
    src/matrix/fem_matrix.cpp
    src/matrix/sparse_matrix.cpp
    src/fem/P1.cpp
    src/fem/poisson.cpp
    src/mesh/adjacency.cpp
    )

target_include_directories(PLNS-solver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/matrix
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mesh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fem
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mesh
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

add_executable(test_fem_matrix
    src/bin/test_fem_matrix.cpp
)
target_link_libraries(test_fem_matrix PRIVATE PLNS-solver)

add_executable(test_sparse_matrix
    src/bin/test_sparse_matrix.cpp
)
target_link_libraries(test_sparse_matrix PRIVATE PLNS-solver)

add_executable(test_poisson
    src/bin/test_poisson.cpp
)
target_link_libraries(test_poisson PRIVATE PLNS-solver)


###############################################################################
# Tests
###############################################################################

enable_testing()

add_test(NAME test_cg            COMMAND test_cg)
add_test(NAME test_fem_matrix    COMMAND test_fem_matrix)
add_test(NAME test_sparse_matrix COMMAND test_sparse_matrix)
add_test(NAME test_poisson       COMMAND test_poisson)

###############################################################################
# Run all tests
###############################################################################

add_custom_target(check

    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure

    DEPENDS
        test_cg
        test_fem_matrix
        test_sparse_matrix
        test_poisson
)