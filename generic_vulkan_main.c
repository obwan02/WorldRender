#include <X11/X.h>
#include <stdio.h>

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "game/core.h"
#include "game/graphics/vulkan_api.h"

// #include "game/game.cpp"
#include "game/graphics/vulkan_api.c"
#include "game/util/clang_assert.c"

static const char* AppName = "World-Scale Renderer";

typedef struct CreateSurfaceData {
	GLFWwindow *window;
} CreateSurfaceData;

//  X11/Win32/Coca    Vulkan Instance
//   |                  v----/
//   V                  |
//  Surface Create Fn   |
//   |                  |
//   V                  /
//  _GVkInit        <--/
static Result createSurface(VkInstance instance, VkSurfaceKHR *out, void *data) {
	CreateSurfaceData *info = data;
	
	VkResult err = glfwCreateWindowSurface(instance, info->window, NULL, out);
	if(err != VK_SUCCESS) {
		log_err("Failed to create vulkan surfance from GLFW window");
		return RESULT_GENERIC_ERR;
	}

	return RESULT_SUCCESS;
}

int main(int argc, const char *argv[]) {

  if (!glfwInit()) {
    puts("Failed to initialise GLFW. Exiting ...");
    return -1;
  }

	// Hint that we are using vulkan, not opengl, so don't 
	// create any opengl context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window = glfwCreateWindow(680, 420, AppName, NULL, NULL);
  glfwMakeContextCurrent(window);

	// Get required vulkan extensions from glfw
	uint32_t extCount;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extCount);


	// Vulkan specific graphics setup
	Result res = _GVkInit(AppName, 0, 0, 1, extCount, extensions); 
	if(res != RESULT_SUCCESS) {
		log_err("Exiting, due to error during Vulkan initialisation. Exiting...");
		exit(-1);
	}


	GDevice device;
	CreateSurfaceData surfaceData = {
		.window = window
	};	

	res = _GVkInitDevice(&device, createSurface, &surfaceData);
	if(res != RESULT_SUCCESS) {
		log_err("Failed to initialise and create logical device. Exiting...");
		exit(-1);
	}

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

	 _GVkCleanup(&device);
	glfwDestroyWindow(window);
	glfwTerminate();

  return 0;
}
