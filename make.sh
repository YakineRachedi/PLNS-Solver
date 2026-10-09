#!/usr/bin/env bash

set -e

echo "====================================="
echo " PLNS-SOLVER build"
echo "====================================="
echo "  ./make.sh seq         -> Sequential"
echo "  ./make.sh omp         -> OpenMP"
echo "  ./make.sh blas        -> Sequential + OpenBLAS in tiny_blas"
echo "  ./make.sh omp-blas    -> OpenMP + OpenBLAS in tiny_blas"
echo "  ./make.sh kokkos      -> Kokkos"
echo "  ./make.sh kokkos-blas -> Kokkos + OpenBLAS in tiny_blas"
echo "====================================="

BACKEND=${1:-seq}
CMAKE_FLAGS=()

case "$BACKEND" in

    seq)
        BUILD="build-seq"
        ;;

    omp)
        BUILD="build-omp"
        CMAKE_FLAGS=(-DUSE_OPENMP=ON)
        ;;

    blas)
        BUILD="build-blas"
        CMAKE_FLAGS=(-DUSE_OPENBLAS_TINY_BLAS=ON)
        ;;

    omp-blas)
        BUILD="build-omp-blas"
        CMAKE_FLAGS=(
            -DUSE_OPENMP=ON
            -DUSE_OPENBLAS_TINY_BLAS=ON
        )
        ;;

    kokkos)
        BUILD="build-kokkos"
        CMAKE_FLAGS=(-DUSE_KOKKOS=ON)
        ;;

    kokkos-blas)
        BUILD="build-kokkos-blas"
        CMAKE_FLAGS=(
            -DUSE_KOKKOS=ON
            -DUSE_OPENBLAS_TINY_BLAS=ON
        )
        ;;

    *)
        echo "Usage: ./make.sh [seq|omp|blas|omp-blas|kokkos|kokkos-blas]"
        exit 1
        ;;
esac

echo
echo "Configuring ($BACKEND)..."

cmake -S . -B "$BUILD" -G Ninja "${CMAKE_FLAGS[@]}"

echo
echo "Building..."

cmake --build "$BUILD" --parallel

echo
echo "Running tests..."

ctest --test-dir "$BUILD" --output-on-failure

echo
echo "Running demos..."

"./$BUILD/demo-poisson"
"./$BUILD/demo-ns"

echo
echo "Build successful."