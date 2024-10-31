rm -rf build
mkdir build
cd build
cmake ..
make

cd ..
build/test/test_cagra cagra.json