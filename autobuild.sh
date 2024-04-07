set -e

if [ ! -d ${PWD}/build ]; then
    mkdir ${PWD}/build
fi

rm -rf ${PWD}/build/*

cd ${PWD}/build &&
    cmake .. &&
    make -j4

cd ..

if [ ! -d ${PWD}/include ]; then
    mkdir ${PWD}/include
fi