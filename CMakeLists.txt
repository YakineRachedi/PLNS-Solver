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
    src/common/chrono.cpp
    src/common/logging.cpp
    src/linalg/conjugate_gradient.cpp
    src/matrix/fem_matrix.cpp
    src/matrix/sparse_matrix.cpp
    src/fem/P1.cpp
    src/fem/poisson.cpp
    src/fem/navier_stokes.cpp
    src/mesh/adjacency.cpp
    src/mesh/cube.cpp
    src/mesh/sphere.cpp
    src/mesh/duplicate_verts.cpp
    )

target_include_directories(PLNS-solver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/matrix
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mesh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fem
)

if (USE_OPENMP AND OpenMP_CXX_FOUND)
	target_compile_options(PLNS-solver PRIVATE -DUSE_OPENMP)
	target_link_libraries(PLNS-solver PUBLIC OpenMP::OpenMP_CXX)
endif()

set(TEST_TARGETS
    test_cg
    test_fem_matrix
    test_sparse_matrix
    test_mesh
)

foreach(test_name ${TEST_TARGETS})
    add_executable(${test_name} src/tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE PLNS-solver)
endforeach()

###############################################################################
# test_poisson depends on USE_FEM_MATRIX -> build it twice, once per variant
###############################################################################

foreach(variant IN ITEMS fem csr)

    if (variant STREQUAL "fem")
        set(matrix_flag 1)
    else()
        set(matrix_flag 0)
    endif()

    set(exe_name test_poisson_${variant})

    add_executable(${exe_name}
        src/fem/poisson.cpp
        src/tests/test_poisson.cpp
    )

    target_link_libraries(${exe_name} PRIVATE PLNS-solver)
    target_compile_definitions(${exe_name} PRIVATE USE_FEM_MATRIX=${matrix_flag})

    add_test(NAME ${exe_name} COMMAND ${exe_name})

endforeach()

###############################################################################
# test_navier_stokes depends on USE_FEM_MATRIX -> build it twice too
###############################################################################

foreach(variant IN ITEMS fem csr)

    if (variant STREQUAL "fem")
        set(matrix_flag 1)
    else()
        set(matrix_flag 0)
    endif()

    set(exe_name test_navier_stokes_${variant})

    add_executable(${exe_name}
        src/fem/navier_stokes.cpp
        src/tests/test_navier_stokes.cpp
    )

    target_link_libraries(${exe_name} PRIVATE PLNS-solver)
    target_compile_definitions(${exe_name} PRIVATE USE_FEM_MATRIX=${matrix_flag})

    add_test(NAME ${exe_name} COMMAND ${exe_name})

endforeach()

###############################################################################
# Tests
###############################################################################

enable_testing()

add_test(NAME test_cg            COMMAND test_cg)
add_test(NAME test_fem_matrix    COMMAND test_fem_matrix)
add_test(NAME test_sparse_matrix COMMAND test_sparse_matrix)
add_test(NAME test_mesh          COMMAND test_mesh)

###############################################################################
# Run all tests
###############################################################################

add_custom_target(check

    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure

    DEPENDS
        test_cg
        test_fem_matrix
        test_sparse_matrix
        test_mesh
        test_poisson_fem
        test_poisson_csr
        test_navier_stokes_fem
        test_navier_stokes_csr

)