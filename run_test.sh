#!/bin/bash

# SPDX-License-Identifier: MIT

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export VASCO_OUTPUT_FILE=/tmp/vasco.out

if [ ! -d "$1" ]; then
    echo "Error: First argument is not the build directory."
    exit 1
fi

BUILD_DIR=$1

if [ ! -f "$BUILD_DIR/bin/testvasco" ]; then
    echo "Error: $BUILD_DIR/bin/testvasco does not exist."
    exit 1
fi

rm /tmp/testvasco-IpcPipe

LD_PRELOAD=$BUILD_DIR/lib/libvascodagammaray.so $BUILD_DIR/bin/testvasco &

sleep 2

echo -n "print_info" | socat - UNIX-CONNECT:/tmp/testvasco-IpcPipe
echo -n "quit" | socat - UNIX-CONNECT:/tmp/testvasco-IpcPipe

sleep 5

if ! pgrep -x "testvasco" > /dev/null; then
    exit 0
else
    echo "Error: testvasco process is still running."
    exit 1
fi
