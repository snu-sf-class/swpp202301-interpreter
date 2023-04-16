#!/bin/bash

cmake -GNinja -Bbuild .
cmake --build build
cp build/swpp-interpreter .
