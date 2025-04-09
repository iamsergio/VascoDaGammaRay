#!/bin/bash

# SPDX-License-Identifier: MIT

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -d "$1" ]; then
    echo "Error: First argument is not the build directory."
    exit 1
fi

BUILD_DIR=$1

if [ ! -f "$BUILD_DIR/bin/testvasco" ]; then
    echo "Error: $BUILD_DIR/bin/testvasco does not exist."
    exit 1
fi

rm -rf /tmp/testvasco-IpcPipe

LD_PRELOAD=$BUILD_DIR/lib/libvascodagammaray.so $BUILD_DIR/bin/testvasco &
