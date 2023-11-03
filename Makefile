C_FLAGS = $$(pkg-config --cflags glfw3 vulkan) -Ivendor/glad/include
LINK_FLAGS = $$(pkg-config --libs glfw3 vulkan)

.PHONY: debug release clean

# Add debug flags when making debug build
debug: bin/game
debug: CXX_FLAGS += -g
debug: C_FLAGS += -g
debug: CXX_FLAGS += -Wall

# Add release flags when doing release build
release: bin/game
release: CXX_FLAGS += -O2

# Linking
bin/game: bin/vulkan.o bin/main.o
	clang++ -o $@ $^ $(LINK_FLAGS)

bin/vulkan.o: vendor/glad/src/vulkan.c
	clang -c -o $@ $^ $(C_FLAGS)

bin/main.o: linux_vulkan_main.c
	clang -c -o $@ $^ $(C_FLAGS) -DWRLD_VULKAN

clean:
	rm bin/*
