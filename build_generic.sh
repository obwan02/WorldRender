#!/bin/bash

mkdir -p bin
clang -c vendor/glad/src/vulkan.c -o bin/vulkan.o -Ivendor/glad/include
clang++ -c generic_vulkan.cpp -Wall -o bin/main.o -std=c++17 -Ivendor/glad/include -DWRLD_VULKAN -gdwarf

clang bin/vulkan.o bin/main.o -o bin/game -lglfw -lvulkan -lstdc++
