cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PLNS-SOLVER LANGUAGES CXX C)


###############################################################################
# Optional HPC backends
###############################################################################

option(USE_OPENMP   "Enable OpenMP backend" OFF)
option(USE_OPENBLAS "Enable OpenBLAS backend" OFF)
option(USE_KOKKOS   "Enable Kokkos backend"  OFF)

if(USE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

if(USE_OPENBLAS)
    find_package(BLAS REQUIRED)
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
# tinyexpr
###############################################################################

add_library(tiny_expr extern/tiny_expr/tinyexpr.c)
target_include_directories(tiny_expr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern)


###############################################################################
# Kokkos (optional)
###############################################################################

if (USE_KOKKOS)

    find_package(Kokkos QUIET)

    if (NOT Kokkos_FOUND)
        message(STATUS "Kokkos not found locally, fetching from GitHub...")

        include(FetchContent)

        FetchContent_Declare(
            kokkos
            GIT_REPOSITORY https://github.com/kokkos/kokkos.git
            GIT_TAG 4.3.01
        )

        set(Kokkos_ENABLE_SERIAL ON CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(kokkos)
    endif()

endif()

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
    src/viewer/camera.cpp
    src/viewer/frustum.cpp
    src/viewer/mouse.cpp
    src/viewer/shaders.cpp
    src/viewer/trackball.cpp
    src/viewer/viewer.cpp
    )

target_include_directories(PLNS-solver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/matrix
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mesh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fem
    ${CMAKE_CURRENT_SOURCE_DIR}/include/viewer
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
)

target_link_libraries(PLNS-solver PUBLIC
	glfw
	imgui_glfw_opengl3
	OpenGL::GL
)

###############################################################################
# Backend selection
###############################################################################

if(USE_OPENMP)
    target_link_libraries(PLNS-solver PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(PLNS-solver PUBLIC USE_OPENMP)
endif()

if(USE_OPENBLAS)
    target_link_libraries(PLNS-solver PUBLIC BLAS::BLAS)
    target_compile_definitions(PLNS-solver PUBLIC USE_OPENBLAS)
endif()

if(USE_KOKKOS)
    target_link_libraries(PLNS-solver PUBLIC Kokkos::kokkos)
    target_compile_definitions(PLNS-solver PUBLIC USE_KOKKOS)
endif()
###############################################################################
# Tests
###############################################################################

enable_testing()

function(plns_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE PLNS-solver)
    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
endfunction()

plns_test(test_cg               src/tests/test_cg.cpp)
plns_test(test_fem_matrix       src/tests/test_fem_matrix.cpp)
plns_test(test_sparse_matrix    src/tests/test_sparse_matrix.cpp)
plns_test(test_mesh             src/tests/test_mesh.cpp)
plns_test(test_mesh_bounds      src/tests/test_mesh_bounds.cpp)
plns_test(test_mesh_io          src/tests/test_mesh_io.cpp)
plns_test(test_mesh_gpu         src/tests/test_mesh_gpu.cpp)
plns_test(test_vec4             src/tests/test_vec4.cpp)
plns_test(test_mat4             src/tests/test_mat4.cpp)
plns_test(test_quat             src/tests/test_quat.cpp)
plns_test(test_geometry         src/tests/test_geometry.cpp)
plns_test(test_rigid_transform  src/tests/test_rigid_transform.cpp)

if(USE_KOKKOS)
    plns_test(test_kokkos_intro src/tests/test_kokkos_intro.cpp)
endif()

target_compile_definitions(test_mesh_io PRIVATE
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

###############################################################################
# Poisson variants
###############################################################################

foreach(variant IN ITEMS fem csr)

    if(variant STREQUAL "fem")
        set(USE_FEM 1)
    else()
        set(USE_FEM 0)
    endif()

    add_executable(test_poisson_${variant}
        src/fem/poisson.cpp
        src/tests/test_poisson.cpp)

    target_link_libraries(test_poisson_${variant} PRIVATE PLNS-solver)
    target_compile_definitions(test_poisson_${variant}
        PRIVATE USE_FEM_MATRIX=${USE_FEM})

    add_test(NAME test_poisson_${variant}
         COMMAND test_poisson_${variant})

endforeach()

###############################################################################
# Navier-Stokes variants
###############################################################################

foreach(variant IN ITEMS fem csr)

    if(variant STREQUAL "fem")
        set(USE_FEM 1)
    else()
        set(USE_FEM 0)
    endif()

    add_executable(test_navier_stokes_${variant}
        src/fem/navier_stokes.cpp
        src/tests/test_navier_stokes.cpp)

    target_link_libraries(test_navier_stokes_${variant} PRIVATE PLNS-solver)
    target_compile_definitions(test_navier_stokes_${variant}
        PRIVATE USE_FEM_MATRIX=${USE_FEM})

    add_test(NAME test_navier_stokes_${variant}
         COMMAND test_navier_stokes_${variant})

endforeach()

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
        test_vec4
        test_mat4
        test_quat
        test_geometry
        test_rigid_transform
        test_poisson_fem
        test_poisson_csr
        test_navier_stokes_fem
        test_navier_stokes_csr
)

if(USE_KOKKOS)
    add_dependencies(check test_kokkos_intro)
endif()

###############################################################################
# Demo
###############################################################################
add_executable(demo-poisson examples/demo-poisson.cpp)
target_compile_options(demo-poisson PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
target_link_libraries(demo-poisson PRIVATE PLNS-solver tiny_expr)

add_executable(demo-ns examples/demo-ns.cpp)
target_compile_options(demo-ns PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
target_link_libraries(demo-ns PRIVATE PLNS-solver tiny_expr)
