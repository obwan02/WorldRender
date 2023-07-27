#!/bin/bash

clang++ macOS_main.cpp  -Wall -o bin/game -std=c++17 -Ivendor/metal-cpp -Ivendor/metal-cpp-extensions -DWRLD_METAL -framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit -gdwarf
