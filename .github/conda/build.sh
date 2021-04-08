#!/bin/bash

mkdir build
cd build

extra_opts=()
if [[ ! -z "${CONDA_BUILD_SYSROOT}" ]]; then
  export MACOSX_DEPLOYMENT_TARGET=10.14
  extra_opts+=("-DCMAKE_OSX_SYSROOT=${CONDA_BUILD_SYSROOT}")
fi

cmake .. "${extra_opts[@]}" \
    -DCMAKE_CXX_COMPILER="${CXX}" \
    -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
    -DCMAKE_INSTALL_LIBDIR="lib" \
    -DCMAKE_BUILD_TYPE=Release

make -j${CPU_COUNT}
make install
