git config --global user.name rudrajyotiroy
git config --global user.email rudrajyotiroy@gmail.com
mkdir -p build && cd build
cmake ..
make
echo "Compile done"
cd ../benchmark1
./run.sh simple
echo "Benchmark 1 done"
cd ../benchmark2
./run.sh anagram
echo "Benchmark 2 done"
cd ../benchmark3
./run.sh compress
echo "Benchmark 3 done"