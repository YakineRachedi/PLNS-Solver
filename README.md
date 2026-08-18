# PLNS-Solver

PLNS is a C++ project focused on the numerical solution of PDEs using the **Finite Element Method**.

The project covers the complete workflow, from mesh processing to the assembly and solution of the resulting linear systems. It is currently developed around two main problems:

- **Poisson equation**
- **Two-dimensional Navier–Stokes equations**

The project is designed as a foundation for experimenting with numerical methods, sparse linear algebra, iterative solvers, and parallel computing.

## Features

- Mesh representation and processing
- Finite Element Method discretization
- P1 finite elements
- Mesh adjacency construction
- Sparse matrix representation using CSR
- FEM-specific matrix representation
- Assembly of mass and stiffness matrices
- Poisson equation solver
- 2D Navier–Stokes solver
- Conjugate Gradient iterative solver
- Optional OpenMP parallelization
- OpenGL-based mesh and solution visualization

## Numerical Workflow

The project follows the complete numerical workflow required to solve a PDE using the Finite Element Method:

```text
                 +------------------+
                 |       Mesh       |
                 | Vertices/Triangles|
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Mesh Processing  |
                 |   Adjacency      |
                 | Geometry data    |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | FEM Discretization|
                 | P1 basis functions|
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Matrix Assembly  |
                 | Mass matrix M    |
                 | Stiffness matrix |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Linear System    |
                 |     A x = b      |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 | Iterative Solver |
                 | Conjugate Gradient|
                 +--------+---------+
                          |
                          v
                 +------------------+
                 |    Solution      |
                 +------------------+
```

The objective is to provide a complete implementation of the path from a geometrical mesh to the numerical solution of a partial differential equation.


## Supported Problems

### Poisson Equation
The project supports the numerical solution of the Poisson equation:

$$-\Delta u = f$$

Using the **Finite Element Method (FEM)**, the continuous problem is discretized into a linear system involving the stiffness matrix and the mass matrix:

$$A u = M f$$

**Where:**
* $A$ is the stiffness matrix.
* $M$ is the mass matrix.
* $f$ is the right-hand side forcing term.
* $u$ is the numerical solution vector.

The resulting linear system is solved using an iterative Conjugate Gradient (CG) algorithm.

---

### Navier–Stokes Equations
The project includes a 2D Navier–Stokes solver based on the **vorticity ($\omega$) and stream-function ($\psi$)** formulation.

**Primary Variables:**
* $\omega$ : Vorticity
* $\psi$ : Stream function

#### Time Integration Workflow
The simulation advances in time through successive time steps:

```text

omega(t)
    |
    v
Compute stream function psi
    |
    v
Compute transport terms
    |
    v
Advance the solution by dt
    |
    v
omega(t + dt)
```

Spatial discretization is performed via the **Finite Element Method (FEM)**. Temporal discretization and numerical integration schemes are handled modularly within the solver architecture to accommodate various flow regimes.

---

## Linear Algebra Engine

The project relies on a custom, lightweight C++ linear algebra library optimized for numerical computing and FEM assembly.

### Data Structures

#### Vectors
Templated vector classes for geometric and algebraic operations:
* `TVec2<T>`: 2D vector representation.
* `TVec3<T>`: 3D vector representation.

#### Dynamic Arrays
* `TArray<T>`: Contiguous dynamic array for high-efficiency memory access.

#### Sparse Matrix Formats
* `CSRMatrix`: Compressed Sparse Row format, optimized for sparse matrix-vector multiplications ($SpMV$).
* `FEMatrix`: FEM-specific matrix representation designed for efficient element assembly.

*Note: The computational pipeline can dynamically switch matrix representations based on algorithm requirements and hardware optimization targets.*

---

## Iterative Solvers

### Conjugate Gradient (CG)
An iterative solver for Symmetric Positive-Definite (SPD) linear systems of the form:

$$A x = b$$

**Algorithm State Vectors:**
* `x` : Current solution vector
* `r` : Residual vector ($r = b - Ax$)
* `p` : Search direction vector
* `Ap` : Matrix-vector product

**Convergence Criterion:**  
The solver iterates until the relative residual satisfies the target tolerance:

$$\frac{\|r\|}{\|b\|} \le \varepsilon_{\text{tol}}$$
