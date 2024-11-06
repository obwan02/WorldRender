#!/bin/bash

gcc ./platform/linux_x86_64.c -o ./bin/game $(pkg-config --static --libs --cflags glfw3 vulkan) -Ivendor/glad/include -I.
