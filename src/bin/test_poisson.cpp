#include <cstdio>
#include <cmath>

#include "P1.h"
#include "poisson.h"

/******************************************************************************
 * Unit tests for PoissonSolver.
 *
 * Mesh used for every test : a unit square divided into two triangles.
 *
 *        3 ----- 2
 *        |     / |
 *        |   /   |
 *        | /     |
 *        0 ----- 1
 *
 * positions :
 *
 *     0 = (0,0,0)
 *     1 = (1,0,0)
 *     2 = (1,1,0)
 *     3 = (0,1,0)
 *
 * triangles :
 *
 *     [0,1,2]
 *     [0,2,3]
 *****************************************************************************/

static bool g_all_passed = true;

static void check(bool cond, const char *name) {
    printf("[%s] %s\n", cond ? "PASS" : "FAIL", name);

    if (!cond)
        g_all_passed = false;
}

static bool almost_equal(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

static void setup_mesh(Mesh & m) {
    m.positions.resize(4);

    m.positions[0] = {0.0f, 0.0f, 0.0f};
    m.positions[1] = {1.0f, 0.0f, 0.0f};
    m.positions[2] = {1.0f, 1.0f, 0.0f};
    m.positions[3] = {0.0f, 1.0f, 0.0f};

    m.indices.push_back(0);
    m.indices.push_back(1);
    m.indices.push_back(2);

    m.indices.push_back(0);
    m.indices.push_back(2);
    m.indices.push_back(3);
}

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

int main() {
    Mesh mesh;
    setup_mesh(mesh);

    printf("PoissonSolver unit tests\n");
    printf("------------------------\n");

    test_initialization(mesh);
    test_zero_mean(mesh);
    test_convergence(mesh);
    test_clear_solution(mesh);

    if (!g_all_passed) {
        printf("\nTEST FAILED\n");
        return 1;
    }

    printf("\nTEST PASSED\n");
    return 0;
}