#!/bin/bash

source .envrc

rm -rf ${BUILD_DIR}

set -e
set -u
set -x

cmake -S . -B ${BUILD_DIR} -G Ninja -D CMAKE_CXX_COMPILER_LAUNCHER=${CCACHE} \
  -D CMAKE_BUILD_TYPE=Release \
  -D CMAKE_SKIP_BUILD_RPATH=OFF \
  -D CMAKE_INSTALL_RPATH=${STAGE_DIR}/lib \
  -D BUILD_WITH_INSTALL_NAME_DIR=OFF \
  -D CMAKE_BUILD_WITH_INSTALL_RPATH=ON \
  -D CMAKE_INSTALL_RPATH_USE_LINK_PATH=ON \
  -D CMAKE_STAGING_PREFIX=${STAGE_DIR} \
  -D CMAKE_PREFIX_PATH=${STAGE_DIR} \
  -D CMAKE_CXX_STANDARD=17 \
  -D BUILD_SHARED_LIBS=OFF -Wdev -Wdeprecated

# build example
cmake --build ${BUILD_DIR} --target all
cmake --build ${BUILD_DIR} --target test

