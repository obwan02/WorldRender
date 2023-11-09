#include <stdio.h>

#include <stdlib.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "game/core.h"
#include "game/graphics/vulkan_api.h"

#include "game/graphics/vulkan_api.c"
#include "game/core.c"

typedef struct CreateSurfaceData {
	GLFWwindow *window;
} CreateSurfaceData;

#define PERM_ARENA_SIZE (4 * 1024 * 1024)
#define SCRATCH_ARENA_SIZE (1 * 1024 * 1024)
static Str app_name = S("World-Scale Renderer");

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

	// TODO: Move away from using libc eventually
	Arena perm_arena = {
		.beg = malloc(PERM_ARENA_SIZE),
	};
	ASSERT(perm_arena.beg);
	perm_arena.end = perm_arena.beg + PERM_ARENA_SIZE;

	Arena scratch = {
		.beg = malloc(SCRATCH_ARENA_SIZE)
	};
	ASSERT(scratch.beg);
	scratch.end = scratch.beg + SCRATCH_ARENA_SIZE;

  if (!glfwInit()) {
    puts("Failed to initialise GLFW. Exiting ...");
    return -1;
  }

	// Hint that we are using vulkan, not opengl, so don't 
	// create any opengl context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window = glfwCreateWindow(680, 420, (const char *) app_name.str, NULL, NULL);
  glfwMakeContextCurrent(window);

	// Get required vulkan extensions from glfw
	u32 ext_cnt;
	const char** exts = glfwGetRequiredInstanceExtensions(&ext_cnt);

	// Vulkan specific graphics setup
	// Ifdef required because apple needs to use MoltenVK
#ifdef __APPLE__
	Result res = _GVkInit(AppName, 0, 0, 1, extCount, extensions, true);
#else
	b32 res = _GVkInit(app_name, 0, 0, 1, ext_cnt, exts, false);
#endif
	if(!res) {
		log_err("Exiting, due to error during Vulkan initialisation...");
		return -1;
	}


	GDevice device;
	CreateSurfaceData surface_data = {
		.window = window
	};	

	GVkDeviceOut dev = _GVkInitDevice(createSurface, &surface_data);
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
