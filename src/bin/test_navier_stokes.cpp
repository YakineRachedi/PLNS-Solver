#include <cstdio>
#include <cmath>

#include "navier_stokes.h"

/******************************************************************************
 * Unit tests for NavierStokesSolver.
 *
 * Mesh used for every test: a unit square divided into two triangles.
 *
 *        3 ----- 2
 *        |     / |
 *        |   /   |
 *        | /     |
 *        0 ----- 1
 *
 * positions:
 *
 *     0 = (0,0,0)
 *     1 = (1,0,0)
 *     2 = (1,1,0)
 *     3 = (0,1,0)
 *
 * triangles:
 *
 *     [0,1,2]
 *     [0,2,3]
 *
 *****************************************************************************/

static bool g_all_passed = true;

static void check(bool cond, const char *name) {
    printf("[%s] %s\n", cond ? "PASS" : "FAIL", name);

    if (!cond)
        g_all_passed = false;
}

static bool almost_equal(double a, double b, double eps = 1e-9) {return std::abs(a - b) < eps;}

/******************************************************************************
 * Create a simple two-triangle mesh.
 *****************************************************************************/
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
 * Compute the discrete integral of a vector V:
 *
 *     integral(V) = sum(M * V)
 *
 * This is used to verify the zero-mean condition.
 *****************************************************************************/

static double discrete_integral(NavierStokesSolver & solver, const double *V) {
    double MV[4];

    solver.M.mvp(V, MV);

    double sum = 0.0;

    for (size_t i = 0; i < solver.N; ++i)
        sum += MV[i];

    return sum;
}


/******************************************************************************
 * Compute the Euclidean norm squared:
 *
 *     ||V||^2 = V^T V
 *****************************************************************************/

static double norm2(const double *V, size_t N) {
    double result = 0.0;

    for (size_t i = 0; i < N; ++i)
        result += V[i] * V[i];

    return result;
}


/******************************************************************************
 * Test solver initialization.
 *
 * The mesh contains four vertices, therefore all unknown vectors must have
 * four degrees of freedom.
 *
 * The unit square has an area equal to one.
 *****************************************************************************/

static void test_initialization(const Mesh & mesh) {
    NavierStokesSolver solver(mesh);

    check(solver.N == 4,
          "Navier-Stokes: number of degrees of freedom");

    check(almost_equal(solver.vol, 1.0),
          "Navier-Stokes: mesh area is correct");

    check(almost_equal(solver.t, 0.0),
          "Navier-Stokes: initial time is zero");

    check(!solver.inited,
          "Navier-Stokes: solver is initially not initialized");
}


/******************************************************************************
 * Test zero-mean correction.
 *
 * After:
 *
 *     set_zero_mean(V)
 *
 * the discrete integral must satisfy:
 *
 *     sum(M * V) = 0
 *****************************************************************************/
static void test_zero_mean(const Mesh & mesh) {
    NavierStokesSolver solver(mesh);

    double V[4] = {1.0, 2.0, 3.0, 4.0};

    solver.set_zero_mean(V);

    double integral = discrete_integral(solver, V);

    check(almost_equal(integral, 0.0),
          "Navier-Stokes: set_zero_mean produces zero integral");
}


/******************************************************************************
 * Test stream-function solver.
 *
 * We start with a non-zero vorticity omega and solve:
 *
 *     S * psi = M * omega
 *
 * The test verifies the residual:
 *
 *     r = M * omega - S * psi
 *
 * Since the stiffness matrix has a constant null space, we first impose a
 * zero mean on omega.
 *****************************************************************************/

static void test_stream_function(const Mesh & mesh) {
    NavierStokesSolver solver(mesh);

    solver.omega[0] =  1.0;
    solver.omega[1] = -1.0;
    solver.omega[2] =  2.0;
    solver.omega[3] = -2.0;

    /* Make omega compatible with the Poisson problem. */
    solver.set_zero_mean(solver.omega.data);

    /* Solve S * psi = M * omega. */
    solver.compute_stream_function();

    double Momega[4];
    double Spsi[4];

    solver.M.mvp(solver.omega.data, Momega);
    solver.S.mvp(solver.psi.data, Spsi);

    double residual2 = 0.0;
    double rhs2 = 0.0;

    for (size_t i = 0; i < solver.N; ++i) {
        double r = Momega[i] - Spsi[i];

        residual2 += r * r;
        rhs2 += Momega[i] * Momega[i];
    }

    double relative_residual =
        std::sqrt(residual2 / rhs2);

    check(relative_residual <= solver.tol * 10.0,
          "Navier-Stokes: stream-function equation is solved");
}


/******************************************************************************
 * Test transport conservation.
 *
 * For one triangle, the transport contributions are:
 *
 *     T_a = s * (psi_b - psi_c) / 6
 *     T_b = s * (psi_c - psi_a) / 6
 *     T_c = s * (psi_a - psi_b) / 6
 *
 * Their sum is exactly:
 *
 *     T_a + T_b + T_c = 0
 *
 * Therefore, globally:
 *
 *     sum(T) = 0
 *
 * up to floating-point round-off errors.
 *****************************************************************************/

static void test_transport_conservation(const Mesh & mesh) {
    NavierStokesSolver solver(mesh);

    solver.omega[0] =  1.0;
    solver.omega[1] = -2.0;
    solver.omega[2] =  3.0;
    solver.omega[3] = -1.0;

    solver.psi[0] =  0.0;
    solver.psi[1] =  1.0;
    solver.psi[2] = -1.0;
    solver.psi[3] =  2.0;

    double T[4];

    solver.compute_transport(T);

    double sum = 0.0;

    for (size_t i = 0; i < solver.N; ++i)
        sum += T[i];

    check(almost_equal(sum, 0.0),
          "Navier-Stokes: transport term preserves total vorticity");
}


/******************************************************************************
 * Test one complete time step.
 *
 * The test performs:
 *
 *     1. S * psi = M * omega
 *     2. Compute transport
 *     3. Solve:
 *
 *        (M + nu * dt * S) omega_new
 *        =
 *        M * omega + dt * T
 *
 *     4. Enforce zero mean
 *     5. Update time
 *
 * We verify that the simulation time has advanced by dt and that the final
 * vorticity has zero discrete mean.
 *****************************************************************************/
static void test_time_step(const Mesh & mesh) {
    NavierStokesSolver solver(mesh);

    solver.omega[0] =  1.0;
    solver.omega[1] = -1.0;
    solver.omega[2] =  2.0;
    solver.omega[3] = -2.0;

    solver.set_zero_mean(solver.omega.data);

    const double dt = 0.01;
    const double nu = 0.1;

    solver.time_step(dt, nu);

    check(almost_equal(solver.t, dt),
          "Navier-Stokes: time is advanced by dt");

    double integral =
        discrete_integral(solver, solver.omega.data);

    check(almost_equal(integral, 0.0, 1e-8),
          "Navier-Stokes: vorticity has zero mean after time step");
}


/******************************************************************************
 * Test diffusion.
 *
 * With a positive viscosity:
 *
 *     nu > 0
 *
 * viscosity should smooth the vorticity field.
 *
 * We perform several time steps and verify that the Euclidean norm of omega
 * does not increase.
 *
 * This is a simple stability/sanity test rather than a strict mathematical
 * proof of energy dissipation.
 *****************************************************************************/

static void test_diffusion(const Mesh & mesh) {
    NavierStokesSolver solver(mesh);

    solver.omega[0] =  1.0;
    solver.omega[1] = -1.0;
    solver.omega[2] =  1.0;
    solver.omega[3] = -1.0;

    solver.set_zero_mean(solver.omega.data);

    double initial_norm2 =
        norm2(solver.omega.data, solver.N);

    const double dt = 0.01;
    const double nu = 1.0;

    for (size_t i = 0; i < 10; ++i)
        solver.time_step(dt, nu);

    double final_norm2 =
        norm2(solver.omega.data, solver.N);

    check(final_norm2 <= initial_norm2 + 1e-10,
          "Navier-Stokes: viscosity does not increase vorticity norm");
}


int main() {
    Mesh mesh;
    setup_mesh(mesh);

    printf("NavierStokesSolver unit tests\n");
    printf("-----------------------------\n");

    test_initialization(mesh);
    test_zero_mean(mesh);
    test_stream_function(mesh);
    test_transport_conservation(mesh);
    test_time_step(mesh);
    test_diffusion(mesh);

    if (!g_all_passed) {
        printf("\nTEST FAILED\n");
        return 1;
    }

    printf("\nTEST PASSED\n");
    return 0;
}