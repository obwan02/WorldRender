#include <stdio.h>

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "game/core.h"
#include "game/graphics/vulkan_api.h"

#include "game/graphics/vulkan_api.c"
#include "game/util/clang_assert.c"
#include "game/core.c"

typedef struct CreateSurfaceData {
	GLFWwindow *window;
} CreateSurfaceData;

static Str appName = S("World-Scale Renderer");

//  X11/Win32/Coca    Vulkan Instance
//   |                  v----/
//   V                  |
//  Surface Create Fn   |
//   |                  |
//   V                  /
//  _GVkInit        <--/
static b32 createSurface(VkInstance instance, void *data, VkSurfaceKHR *out) {
	CreateSurfaceData *info = data;
	
	VkResult err = glfwCreateWindowSurface(instance, info->window, NULL, out);
	if(err != VK_SUCCESS) {
		log_err("Failed to create vulkan surfance from GLFW window");
		return false;
	}

	return true;
}

int main(int argc, const char *argv[]) {

  if (!glfwInit()) {
    puts("Failed to initialise GLFW. Exiting ...");
    return -1;
  }

	// Hint that we are using vulkan, not opengl, so don't 
	// create any opengl context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window = glfwCreateWindow(680, 420, (const char *) appName.str, NULL, NULL);
  glfwMakeContextCurrent(window);

	// Get required vulkan extensions from glfw
	uint32_t extCount;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extCount);


	// Vulkan specific graphics setup
	// Ifdef is necessary evil for now, as long as this "generic" 
	// main file exists.
#ifdef __APPLE__
	Result res = _GVkInit(AppName, 0, 0, 1, extCount, extensions, true);
#else
	b32 res = _GVkInit(appName, 0, 0, 1, extCount, extensions, false);
#endif
	if(!res) {
		log_err("Exiting, due to error during Vulkan initialisation...");
		return -1;
	}


	GDevice device;
	CreateSurfaceData surfaceData = {
		.window = window
	};	

	GVkDeviceOut dev = _GVkInitDevice(createSurface, &surfaceData);
	if(dev.err) {
		log_err("Failed to initialise and create logical device. Exiting...");
		return -1;
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
