echo "Building a demo of the project"
echo "-----------------------------------------------"

cmake -B build -G Ninja
cmake --build build --parallel 4
./build/demo-poisson
./build/demo-ns

echo "-----------------------------------------------"
echo "Build is successful! You can find the demo in build/demo-poisson and build/demo-ns"