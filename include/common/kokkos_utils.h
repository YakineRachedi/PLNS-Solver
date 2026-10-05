#pragma once

#ifdef USE_KOKKOS
#include <Kokkos_Core.hpp>

struct KokkosScope {
    KokkosScope(int& argc, char**& argv) {
        Kokkos::initialize(argc, argv);
    }

    ~KokkosScope() {
        Kokkos::finalize();
    }
};
#else

struct KokkosScope {
    KokkosScope(int&, char**&) {}
};

#endif