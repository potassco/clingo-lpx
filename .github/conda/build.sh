#!/bin/bash

mkdir build
cd build

cmake .. \
    -DCMAKE_CXX_COMPILER="${CXX}" \
    -DPython_ROOT_DIR="${PREFIX}" \
    -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
    -DCMAKE_INSTALL_LIBDIR="lib" \
    -DCLINGOLPX_MANAGE_RPATH=Off \
    -DPYCLINGOLPX_ENABLE="require" \
    -DCMAKE_BUILD_TYPE=Release

make -j${CPU_COUNT}
make install