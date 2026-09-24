#include <iostream>
#include <cstdio>
#include <Kokkos_Core.hpp>

int main(int argc, char *argv[]) {
    Kokkos::initialize(argc, argv);
    {
        Kokkos::print_configuration(std::cout);

        const int N = 10;

        Kokkos::View<double*> x("x", N);
        Kokkos::View<double*> y("y", N);

        Kokkos::parallel_for("init", N, KOKKOS_LAMBDA(const int i) {
            x(i) = 1.0;
            y(i) = 2.0;
        });

        /* AXPY: y = 3*x + y */
        double a = 3.0;
        Kokkos::parallel_for("axpy", N, KOKKOS_LAMBDA(const int i) {
            y(i) += a * x(i);
        });

        /* Dot product via parallel_reduce */
        double result = 0.0;
        Kokkos::parallel_reduce("dot", N,
            KOKKOS_LAMBDA(const int i, double &lsum) {
                lsum += x(i) * y(i);
            }, result);

        printf("dot(x,y) = %f (expected %f)\n", result, N * 1.0 * 5.0);
    }
    Kokkos::finalize();
    return 0;
}