#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd $SCRIPT_DIR

BUILD_DIR=build-rel
if [ -n "$VASCO_BUILD_DIR" ]; then
    BUILD_DIR=$VASCO_BUILD_DIR
fi

mkdir -p $BUILD_DIR
cmake --preset rel -B $BUILD_DIR
cmake --build $BUILD_DIR
