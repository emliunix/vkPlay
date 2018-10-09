#/bin/bash

ROOT_DIR=$(cd `dirname $0`;pwd)
cd $ROOT_DIR

if [ ! -e build ]; then
  mkdir build
fi

cd build
# -march=native to enable popcnt feature
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_FLAGS="-march=native" -DCMAKE_CXX_FLAGS="-march=native" ..
make
