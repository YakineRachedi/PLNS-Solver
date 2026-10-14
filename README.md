# PLNS-Solver

**PLNS-Solver** is a C++17 project for experimenting with the numerical solution of partial differential equations (PDEs), with a focus on the **Finite Element Method (FEM)**, sparse linear algebra, iterative solvers, and CPU-parallel computing.

The current application examples cover a Poisson problem and a two-dimensional incompressible Navier–Stokes formulation using vorticity and stream function. The repository also contains geometry and mesh utilities, a small linear-algebra layer, automated tests, and an OpenGL-based viewer for visualizing meshes and scalar fields.

> **Project status:** research/learning code that is under active development. Numerical results and performance depend on the selected discretization, mesh, compiler, libraries, and backend.

## Contents

- [Features](#features)
- [Numerical workflow](#numerical-workflow)
- [PDE solvers](#pde-solvers)
- [Linear algebra](#linear-algebra)
- [Compute backends](#compute-backends)
- [Visualization](#visualization)
- [Requirements](#requirements)
- [Build and test](#build-and-test)
- [Project structure](#project-structure)
- [Dependencies and attribution](#dependencies-and-attribution)
- [License](#license)

## Features

### Numerical methods

- P1 finite-element discretization on triangular meshes.
- Assembly of finite-element mass and stiffness matrices.
- Poisson problem solver.
- Two-dimensional incompressible Navier–Stokes solver in vorticity–stream-function form.
- Conjugate Gradient (CG) iterative solver for suitable symmetric positive-definite systems.
- Residual-based convergence checks and solver-state management.
- Zero-mean correction for fields associated with a constant null space.

### Mesh processing and geometry

- Triangle-mesh representation, including vertex positions and connectivity.
- Mesh loading and generation utilities, including cube and sphere meshes.
- Vertex deduplication and adjacency construction.
- Mesh bounding-box computation.
- Geometry helpers for vectors, matrices, quaternions, rigid transforms, rays, planes, spheres, and triangle normals.

### Linear algebra and data structures

- `TArray<T>`: contiguous dynamic array.
- Templated vector and matrix types, including `TVec2<T>`, `TVec3<T>`, `TVec4<T>`, `TMat3<T>`, and `TMat4<T>`.
- `CSRMatrix`: Compressed Sparse Row representation for sparse matrix operations.
- `FEMatrix`: finite-element-specific storage formats (`P1_cst`, `P1_sym`, and `P1_gen`).
- Sparse matrix–vector products and matrix-coefficient summation.
- A lightweight `tiny_blas` interface for vector copy, AXPY, AXPBY, dot product, and sum operations.

### Parallel computing

- Sequential C++ implementation as the default.
- Optional OpenMP implementations for supported loops and reductions.
- Optional Kokkos implementations for supported operations.
- Optional OpenBLAS calls for the supported operations in `tiny_blas`.
- OpenBLAS can be enabled independently of the main backend; for example, OpenMP can be used in matrix operations while OpenBLAS handles selected `tiny_blas` operations.

### Visualization and interactive demos

- OpenGL-based mesh and scalar-field visualization.
- GLFW window/context management and ImGui controls.
- Poisson and Navier–Stokes demo applications.
- Editable right-hand-side expression in the Poisson demo through TinyExpr.

## Numerical workflow

The general workflow implemented by the project is:

```text
Triangle mesh
    |
    v
Mesh processing and geometry
    |  connectivity, adjacency, geometric data
    v
P1 finite-element discretization
    |  local element contributions
    v
Assembly of global operators
    |  mass matrix M, stiffness matrix K
    v
Discrete linear system
    |  K u = b (or the corresponding problem-specific system)
    v
Iterative linear solver
    |  Conjugate Gradient where applicable
    v
Numerical solution and visualization
```

## PDE solvers

### Poisson equation

The Poisson example considers a problem of the form

\[
-\Delta u = f.
\]

With a finite-element discretization, this leads to a linear system involving the stiffness matrix and a discrete right-hand side. Depending on how the forcing term is represented, the right-hand side can be assembled using the mass matrix, for example as \(b = M f\):

\[
K u = b.
\]

Here, \(K\) is the stiffness matrix, \(M\) is the mass matrix, \(f\) is the forcing term, and \(u\) is the vector of nodal solution values. The system is solved iteratively using Conjugate Gradient when the matrix satisfies the required assumptions.

### Two-dimensional Navier–Stokes equations

The Navier–Stokes implementation uses the **vorticity** \(\omega\) and **stream function** \(\psi\) formulation. The stream function is recovered from a Poisson-type equation, and the vorticity is advanced through a transport/diffusion time step.

The main variables are:

- \(\omega\): vorticity;
- \(\psi\): stream function;
- \(\nu\): kinematic viscosity;
- \(\Delta t\): time-step size.

At a high level, the time-stepping workflow is:

```text
Current vorticity omega(t)
          |
          v
Compute the stream function psi
          |
          v
Assemble/evaluate transport contributions
          |
          v
Apply the time-step and diffusion terms
          |
          v
Obtain the updated vorticity omega(t + dt)
```

The implementation includes mass and stiffness operators, transport-term assembly, time advancement, and zero-mean correction. The exact formulation and numerical behavior depend on the selected matrix representation and solver settings.

## Linear algebra

### Matrix representations

`CSRMatrix` stores a sparse matrix in Compressed Sparse Row format, using row offsets, column indices, and coefficient values. It is intended for general sparse matrix operations, including sparse matrix–vector multiplication.

`FEMatrix` exploits the structure of P1 finite-element matrices. Its storage mode controls how off-diagonal coefficients are represented:

- `P1_cst`: one shared off-diagonal coefficient per triangle;
- `P1_sym`: three symmetric edge coefficients per triangle;
- `P1_gen`: six independent directed off-diagonal coefficients per triangle.

The code provides dedicated matrix–vector products and coefficient-sum operations for these formats.

### Conjugate Gradient

The CG solver targets linear systems

\[
A x = b
\]

where \(A\) must be symmetric positive-definite for standard CG convergence guarantees. It maintains the solution, residual, search direction, and matrix–vector product, and stops according to a configured convergence tolerance.

### `tiny_blas`

The lightweight vector API currently includes:

| Routine | Operation |
|---|---|
| `blas_copy(src, dest, N)` | Copy `src` into `dest` |
| `blas_axpy(a, x, y, N)` | \(y \leftarrow ax + y\) |
| `blas_axpby(a, x, b, y, N)` | \(y \leftarrow ax + by\) |
| `blas_dot(x, y, N)` | Dot product \(x^T y\) |
| `blas_sum(x, N)` | Sum all elements of `x` |

OpenBLAS is used only for routines explicitly dispatched to CBLAS. In the current design, `blas_sum` continues to use Kokkos, OpenMP, or the sequential fallback; it is not a CBLAS call.

## Compute backends

The main compute backend is selected at configure time. **OpenMP and Kokkos are mutually exclusive in the current CMake configuration.** OpenBLAS for `tiny_blas` is an independent optional feature.

| Configuration | Main matrix/FEM operations | Supported `tiny_blas` routines |
|---|---|---|
| Default | Sequential | Sequential |
| OpenMP | OpenMP where implemented | OpenMP |
| Kokkos | Kokkos where implemented | Kokkos |
| OpenBLAS only | Sequential | OpenBLAS where implemented; `blas_sum` remains sequential |
| OpenMP + OpenBLAS | OpenMP where implemented | OpenBLAS where implemented; `blas_sum` uses OpenMP |
| Kokkos + OpenBLAS | Kokkos where implemented | OpenBLAS where implemented; `blas_sum` uses Kokkos |

OpenBLAS does **not** automatically parallelize `CSRMatrix`, `FEMatrix`, or other code that has no OpenBLAS implementation. The OpenBLAS option affects the operations in `tiny_blas` that explicitly call CBLAS.

The Kokkos configuration currently included in the project enables its Serial backend when Kokkos is fetched from source. This is useful for testing the Kokkos programming path, but it does not by itself provide CPU multithreading. Additional Kokkos backends require suitable Kokkos configuration and compatible data accessibility; in particular, raw host pointers cannot be assumed to be directly accessible from a GPU backend.

## Visualization

The project uses OpenGL for rendering, GLFW for window and context creation, and ImGui for the interactive interface. The demos can display the mesh and computed scalar field and expose controls for running the numerical solver.

- `demo-poisson`: interactive Poisson example.
- `demo-ns`: Navier–Stokes time-stepping example.

A working OpenGL-capable environment is required to run the graphical demos. On headless machines or in some WSL environments, the numerical tests may be usable even when an interactive OpenGL window is not available.

## Requirements

Core build requirements:

- CMake 3.14 or newer;
- C and C++ compilers, with C++17 support;
- Ninja (used in the examples below);
- OpenGL development libraries;
- the platform build dependencies needed by GLFW.

Optional features:

- OpenMP-capable compiler/runtime for `USE_OPENMP`;
- Kokkos for `USE_KOKKOS` (CMake can fetch the configured version if it is not found locally and network access is available);
- OpenBLAS development files and `pkg-config` for `USE_OPENBLAS_TINY_BLAS` in the Linux/WSL CMake setup.

GLFW and ImGui are included in `extern/`. TinyExpr is also included and is used by the Poisson demo to evaluate a user-provided right-hand-side expression.

## Build and test
The project uses CMake and Ninja for configuration and compilation. A helper script, make.sh, is also provided to automate the build process, run the test suite, and execute the demonstration programs.

Run commands from the repository root. Use a separate build directory for each backend combination so that CMake options do not leak between configurations.

### Using the helper script

#### Sequential backend (default)

./make.sh or ./make.sh seq

#### OpenMP backend

./make.sh omp

#### Sequential + OpenBLAS

./make.sh blas

#### OpenMP + OpenBLAS

./make.sh omp-blas

#### Kokkos backend

./make.sh kokkos

#### Kokkos + OpenBLAS

./make.sh kokkos-blas

The script automatically configures and builds the selected backend, runs the test suite using CTest, and executes the Poisson and Navier–Stokes demonstration programs. Each configuration uses a separate build directory.

### Manual configuration with CMake

#### Sequential backend

```bash
cmake -S . -B build-seq -G Ninja
cmake --build build-seq --parallel
ctest --test-dir build-seq --output-on-failure
```

#### OpenMP backend

```bash
cmake -S . -B build-omp -G Ninja -DUSE_OPENMP=ON
cmake --build build-omp --parallel
ctest --test-dir build-omp --output-on-failure
```

#### Kokkos backend

```bash
cmake -S . -B build-kokkos -G Ninja -DUSE_KOKKOS=ON
cmake --build build-kokkos --parallel
ctest --test-dir build-kokkos --output-on-failure
```

#### OpenBLAS for `tiny_blas`, with the sequential main backend

```bash
cmake -S . -B build-blas -G Ninja -DUSE_OPENBLAS_TINY_BLAS=ON
cmake --build build-blas --parallel
ctest --test-dir build-blas --output-on-failure
```

#### OpenMP + OpenBLAS

```bash
cmake -S . -B build-omp-blas -G Ninja \
  -DUSE_OPENMP=ON \
  -DUSE_OPENBLAS_TINY_BLAS=ON
cmake --build build-omp-blas --parallel
ctest --test-dir build-omp-blas --output-on-failure
```

#### Kokkos + OpenBLAS

```bash
cmake -S . -B build-kokkos-blas -G Ninja \
  -DUSE_KOKKOS=ON \
  -DUSE_OPENBLAS_TINY_BLAS=ON
cmake --build build-kokkos-blas --parallel
ctest --test-dir build-kokkos-blas --output-on-failure
```

> **CMake note:** the `USE_OPENBLAS_TINY_BLAS` target-linking and compile-definition block must appear **after** `add_library(PLNS-solver ...)`. The target must exist before `target_link_libraries()` or `target_compile_definitions()` can refer to it. The Linux/WSL example also assumes that `pkg-config` can find OpenBLAS.

### Run the demos

After a successful build, run a demo from its build directory or from the repository root using its path:

```bash
./build-seq/demo-poisson
./build-seq/demo-ns
```

Replace `build-seq` with the build directory you configured.

## Tests

The project uses CTest to register unit and solver tests. Current test targets cover areas including:

- Conjugate Gradient;
- FEM and CSR matrix operations;
- mesh creation, bounds, I/O, and GPU-mesh setup;
- vector, matrix, quaternion, geometry, and rigid-transform utilities;
- Poisson solver with FEM and CSR matrix representations;
- Navier–Stokes solver with FEM and CSR matrix representations;
- an additional Kokkos introduction test when Kokkos is enabled.

Run the tests with:

```bash
ctest --test-dir build-seq --output-on-failure
```

Change the directory name to match the configuration being tested. Some mesh/GPU-related tests may require a valid graphics environment, depending on how they initialize OpenGL.

## Project structure

The following is a high-level overview of the main directories and targets:

```text
PLNS-Solver/
├── README.md
├── CMakeLists.txt
├── make.sh
├── .github/ci.yml
├── include/
│   ├── common/       # Common utilities and timing/logging
│   ├── linalg/       # Iterative solver and linear algebra interfaces
│   ├── matrix/       # CSRMatrix and FEMatrix interfaces
│   ├── mesh/         # Mesh and geometry interfaces
│   ├── fem/          # Poisson and Navier–Stokes interfaces
│   └── viewer/       # Camera, viewer, shader, and input interfaces
├── src/
│   ├── common/
│   ├── linalg/
│   ├── matrix/
│   ├── fem/
│   ├── mesh/
│   ├── viewer/
│   └── tests/
├── examples/         # Interactive Poisson and Navier–Stokes demos
├── extern/           # Bundled third-party dependencies
└── data/             # Test/example data, if present
```

This tree is schematic; exact filenames can vary as the project evolves.

## Dependencies and attribution

The initial motivation for this project came from an academic problem studied during my coursework at Sorbonne University. While the original problem and some of the underlying concepts were inspired by the material covered in these courses, the implementation presented in this repository was developed independently.

The project has since evolved significantly beyond its initial academic scope, incorporating numerical methods, scientific computing techniques, and programming skills acquired through other courses. This development includes the implementation and adaptation of solver components to the available computing backends.

I would like to acknowledge the professors at Sorbonne University whose courses provided valuable theoretical foundations, mathematical tools, and computational concepts that helped shape this project: Didier Smith, Émile Parolin, Xavier Claeys, Nathalie Ayi, Albert Cohen, Sidi-Mahmoud Kaber, and Cindy Guichard.

The repository also includes or uses third-party components, including GLFW, ImGui, TinyExpr, Stbi and FastObj. These components remain subject to their own copyright notices and license terms. Keep the notices bundled in `extern/` and check the licenses of externally installed dependencies before redistributing source or binaries. The top-level license applies to the original project code authored for PLNS-Solver; it does not replace third-party licenses.

## License

The original PLNS-Solver code authored for this project is released under the **MIT License**. See [`LICENSE`](LICENSE) for the complete text. Third-party dependencies remain under their respective licenses.
