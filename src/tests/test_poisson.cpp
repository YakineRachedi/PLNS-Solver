#include "test_utils.h"
#include "test_meshes.h"
#include "cube.h"
#include "P1.h"
#include "poisson.h"


/******************************************************************************
 * Test mesh and solver initialization.
 *
 * The mesh contains 4 vertices, therefore the Poisson problem must have
 * 4 degrees of freedom.
 *****************************************************************************/
static void test_initialization(const Mesh & mesh) {
    PoissonSolver solver(mesh);

    check(solver.N == 4,
          "Poisson: number of degrees of freedom");

    check(almost_equal(solver.vol, 1.0),
          "Poisson: mesh surface is correct");

    check(!solver.inited,
          "Poisson: solver is not initialized initially");

    check(!solver.converged,
          "Poisson: solver is not converged initially");
}

/******************************************************************************
 * Test zero mean constraint.
 *
 * The Poisson problem solved here has a constant null space. The solver
 * therefore removes the mean value of vectors using set_zero_mean().
 *
 * We test that:
 *
 *     integral(V) = 0
 *
 * after applying the zero mean correction.
 *****************************************************************************/
static void test_zero_mean(const Mesh & mesh) {
    PoissonSolver solver(mesh);

    double V[4] = {1.0, 2.0, 3.0, 4.0};

    solver.set_zero_mean(V);

    /*
     * The discrete integral is computed using the mass matrix:
     *
     *     integral(V) = sum(M * V)
     */
    double MV[4];
    solver.M.mvp(V, MV);

    double integral = 0.0;
    for (size_t i = 0; i < solver.N; ++i)
        integral += MV[i];

    check(almost_equal(integral, 0.0),
          "Poisson: set_zero_mean produces zero integral");
}

/******************************************************************************
 * Test solution of the Poisson problem.
 *
 * Use a right-hand side with zero mean:
 *
 *     f = {1, -1, 1, -1}
 *
 * The solver should converge and produce a small relative residual.
 *****************************************************************************/
static void test_convergence(const Mesh & mesh) {
    PoissonSolver solver(mesh);

    solver.f[0] =  1.0;
    solver.f[1] = -1.0;
    solver.f[2] =  1.0;
    solver.f[3] = -1.0;

    solver.do_iterate(100, 1e-10);

    check(solver.converged,
          "Poisson: solver converges");

    check(solver.rel_error <= 1e-10,
          "Poisson: relative residual is below tolerance");

    check(solver.iterate > 0,
          "Poisson: at least one CG iteration was performed");
}

/******************************************************************************
 * Test solution reset.
 *
 * After clear_solution():
 *
 *     u = 0
 *
 * and the solver state must be reset.
 *****************************************************************************/
static void test_clear_solution(const Mesh & mesh) {
    PoissonSolver solver(mesh);

    solver.f[0] =  1.0;
    solver.f[1] = -1.0;
    solver.f[2] =  1.0;
    solver.f[3] = -1.0;

    solver.do_iterate(100, 1e-10);

    solver.clear_solution();

    bool solution_is_zero = true;

    for (size_t i = 0; i < solver.N; ++i) {
        if (!almost_equal(solver.u[i], 0.0))
            solution_is_zero = false;
    }

    check(solution_is_zero,
          "Poisson: clear_solution resets solution to zero");

    check(!solver.converged,
          "Poisson: clear_solution resets convergence state");

    check(solver.iterate == 0,
          "Poisson: clear_solution resets iteration count");
}


/******************************************************************************
 * Test Poisson solver on a triangulated surface embedded in 3D.
 *
 * A subdivided cube is used as the computational domain.
 *
 * The mesh is a closed triangulated surface embedded in R^3.
 * This test validates that:
 *
 *     - the mesh can be used to construct a PoissonSolver;
 *     - the FEM matrices are assembled correctly;
 *     - the Conjugate Gradient solver converges;
 *     - the final residual satisfies the requested tolerance.
 *
 *****************************************************************************/
static void test_convergence_3d() {
    Mesh mesh;

    /*
     * Build a cube surface with several subdivisions.
     *
     * This produces a larger problem than the unit square used by the
     * previous tests.
     */
    int res = load_cube(mesh, 3);

    check(res == 0,
          "Poisson 3D: cube mesh creation succeeds");

    if (res != 0)
        return;

    PoissonSolver solver(mesh);

    /*
     * Use a non-constant right-hand side.
     *
     * A constant right-hand side would be removed by the zero-mean
     * correction performed by the solver.
     */
    for (size_t i = 0; i < solver.N; ++i) {
        solver.f[i] =
            std::sin(0.5 * (double)i) +
            std::cos(0.7 * (double)i);
    }

    /*
     * Solve:
     *
     *     A * u = M * f
     */
    solver.do_iterate(10000, 1e-10);

    check(solver.converged,
          "Poisson 3D: solver converges");

    check(solver.rel_error <= 1e-10,
          "Poisson 3D: relative residual is below tolerance");

    check(solver.iterate > 0,
          "Poisson 3D: at least one CG iteration was performed");

    /*
     * Verify that the computed solution contains valid finite values.
     */
    bool finite_solution = true;

    for (size_t i = 0; i < solver.N; ++i) {
        if (!std::isfinite(solver.u[i])) {
            finite_solution = false;
            break;
        }
    }

    check(finite_solution,
          "Poisson 3D: solution contains only finite values");
}

int main() {
    Mesh mesh;
    setup_unit_square_mesh(mesh);

    printf("PoissonSolver unit tests\n");
    printf("------------------------\n");

    test_initialization(mesh);
    test_zero_mean(mesh);
    test_convergence(mesh);
    test_clear_solution(mesh);

    test_convergence_3d();

    if (!g_all_passed) {
        printf("\nTEST FAILED\n");
        return 1;
    }

    printf("\nTEST PASSED\n");
    return 0;
}