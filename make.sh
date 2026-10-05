#!/usr/bin/env bash

set -e

echo "====================================="
echo " PLNS-SOLVER build"
echo "====================================="
echo "  ./make.sh          -> Sequential"
echo "  ./make.sh omp      -> OpenMP"
echo "  ./make.sh blas     -> OpenBLAS"
echo "  ./make.sh kokkos   -> Kokkos"
echo "====================================="

BACKEND=${1:-seq}

case $BACKEND in
    seq)
        BUILD=build-seq
        CMAKE_FLAGS=""
        ;;
    omp)
        BUILD=build-omp
        CMAKE_FLAGS="-DUSE_OPENMP=ON"
        ;;
    blas)
        BUILD=build-blas
        CMAKE_FLAGS="-DUSE_OPENBLAS=ON"
        ;;
    kokkos)
        BUILD=build-kokkos
        CMAKE_FLAGS="-DUSE_KOKKOS=ON"
        ;;
    *)
        echo "Usage: ./make.sh [seq|omp|blas|kokkos]"
        exit 1
        ;;
esac

echo
echo "Configuring ($BACKEND)..."
cmake -S . -B $BUILD -G Ninja $CMAKE_FLAGS

echo
echo "Building..."
cmake --build $BUILD --parallel

echo
echo "Running tests..."
ctest --test-dir $BUILD --output-on-failure

echo
echo "Running demos..."
./$BUILD/demo-poisson
./$BUILD/demo-ns

echo
echo "Build successful."