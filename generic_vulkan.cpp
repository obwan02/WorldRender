#include <stdio.h>
#include <GLFW/glfw3.h>
#include "game/core.h"
#include "game/graphics/vulkan_api.h"

// #include "game/game.cpp"
#include "game/graphics/vulkan_api.cpp"

static const char* AppName = "World-Scale Renderer";

int main(int argc, const char *argv[]) {
  if (!glfwInit()) {
    puts("Failed to initialise GLFW. Exiting ...");
    return -1;
  }

  GLFWwindow *window = glfwCreateWindow(680, 420, AppName, nullptr, nullptr);
  glfwMakeContextCurrent(window);

	// Get required vulkan extensions from glfw
	uint32_t extCount;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extCount);

	// Vulkan specific graphics setup
	if(Error err = g::_vkInitInstance(AppName, 0, 0, 1, extCount, extensions); err != NO_ERROR) {
		err->print();
		exit(-1);
	}

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  return 0;
}
