#!/bin/sh

set -x

SOURCE_DIR=$(pwd)
BUILD_DIR=${BUILD_DIR:-./build}
BUILD_TYPE=${BUILD_TYPE:-release}
CXX=${CXX:-g++}

mkdir -p $BUILD_DIR/$BUILD_TYPE-cpp11 &&
    cd $BUILD_DIR/$BUILD_TYPE-cpp11 &&
    cmake \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        $SOURCE_DIR && 
    make $*
