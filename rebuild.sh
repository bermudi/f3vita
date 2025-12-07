#!/usr/bin/env bash
set -e

BUILD_DIR="build"

rm -rf "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"
