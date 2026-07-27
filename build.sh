#!/bin/bash

set -e

COMPILER="g++"
STANDARD="-std=c++14"
FLAGS="-Wall -Wextra -O2"
OUTPUT_DIR="./build"
TARGET="$OUTPUT_DIR/WebServer"

SRC_FILES=$(find . -maxdepth 3 -name "*cpp")

mkdir -p "$OUTPUT_DIR"

if [ -z "$SRC_FILES" ]; then
    echo "No .cpp files were found in the current directory"
    exit 1
fi

echo "Compiling..."

$COMPILER $STANDARD $FLAGS $SRC_FILES -o "$TARGET"

echo "Build successful! Executable: $TARGET"

"$TARGET"