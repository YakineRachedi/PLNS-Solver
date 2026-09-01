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
# OpenGL / GLFW / ImGui (needed by GPUMesh and the graphical demo)
###############################################################################

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

add_subdirectory(extern/glfw)

add_library(imgui_glfw_opengl3
	extern/imgui/imgui.cpp
	extern/imgui/imgui_draw.cpp
	extern/imgui/imgui_tables.cpp
	extern/imgui/imgui_widgets.cpp
	extern/imgui/imgui_impl_glfw.cpp
	extern/imgui/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_glfw_opengl3 PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/include
)

target_link_libraries(imgui_glfw_opengl3 PUBLIC glfw OpenGL::GL)

###############################################################################
# Library
###############################################################################

add_library(PLNS-solver
    src/common/chrono.cpp
    src/common/logging.cpp
    src/common/gl_utils.cpp
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
    src/mesh/mesh_bounds.cpp
    src/mesh/mesh_io.cpp
    src/mesh/mesh_gpu.cpp
    )

target_include_directories(PLNS-solver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/matrix
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mesh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fem
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
)

target_link_libraries(PLNS-solver PUBLIC
	glfw
	imgui_glfw_opengl3
	OpenGL::GL
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
    test_mesh_bounds
    test_mesh_io
    test_mesh_gpu
)

foreach(test_name ${TEST_TARGETS})
    add_executable(${test_name} src/tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE PLNS-solver)
endforeach()

target_compile_definitions(test_mesh_io PRIVATE
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data"
)

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
add_test(NAME test_mesh_bounds   COMMAND test_mesh_bounds)
add_test(NAME test_mesh_io       COMMAND test_mesh_io)
add_test(NAME test_mesh_gpu      COMMAND test_mesh_gpu)

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
        test_mesh_bounds
        test_mesh_io
        test_mesh_gpu
        test_poisson_fem
        test_poisson_csr
        test_navier_stokes_fem
        test_navier_stokes_csr

)