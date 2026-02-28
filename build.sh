#!/bin/sh

mkdir build
cd build
cmake ..
cmake --build .
cp -r ../assets .
