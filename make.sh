#!/bin/sh

set -x

SOURCE_DIR=$(pwd)
BUILD_DIR=${BUILD_DIR:-./build}

if [ ! "$1" ]; then
    BUILD_TYPE=debug
else
    BUILD_TYPE=$1
fi

mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../
make