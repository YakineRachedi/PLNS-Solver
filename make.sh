#!/usr/bin/env bash

set -e

echo "====================================="
echo "  USAGE GUIDE"
echo "====================================="
echo "  # Sequential"
echo "  ./make.sh"
echo ""
echo "  # OpenMP"
echo "  ./make.sh omp"
echo ""
echo "  # OpenBLAS"
echo "  ./make.sh blas"
echo ""
echo "  # Kokkos"
echo "  ./make.sh kokkos"
echo "====================================="
echo

BACKEND=${1:-seq}

echo "====================================="
echo " PLNS-SOLVER ($BACKEND)"
echo "====================================="

case $BACKEND in
    seq)
        cmake -B build -G Ninja
        ;;
    omp)
        cmake -B build -G Ninja -DUSE_OPENMP=ON
        ;;
    blas)
        cmake -B build -G Ninja -DUSE_OPENBLAS=ON
        ;;
    kokkos)
        cmake -B build -G Ninja
        ;;
    *)
        echo "Usage: ./make.sh [seq|omp|blas|kokkos]"
        exit 1
        ;;
esac

cmake --build build --parallel

echo
echo "Running demos..."
./build/demo-poisson
./build/demo-ns

echo
echo "Build successful."