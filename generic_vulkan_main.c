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

static VkSurfaceKHR createSurface(GLFWwindow *window) {
	VkSurfaceKHR surf;
	VkResult err = glfwCreateWindowSurface(_GVkGetInstance(), window, NULL, &surf);

	if(err != VK_SUCCESS) {
		log_err("Failed to create vulkan surfance from GLFW window");
		return VK_NULL_HANDLE;
	}

	return surf;
}

int main(int argc, const char *argv[]) {

	// TODO: Move away from using libc eventually
	Arena scratch_arena = {
		.beg = malloc(SCRATCH_ARENA_SIZE)
	};
	ASSERT(scratch_arena.beg);
	scratch_arena.end = scratch_arena.beg + SCRATCH_ARENA_SIZE;

	Arena perm_arena = {
		.beg = malloc(SCRATCH_ARENA_SIZE)
	};
	ASSERT(perm_arena.beg);
	perm_arena.end = perm_arena.beg + PERM_ARENA_SIZE;

  if (!glfwInit()) {
    log_err("Failed to initialise GLFW. Exiting ...");
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
	b32 res = _GVkInit(app_name, 0, 0, 1, ext_cnt, exts, true, scratch);
#else
	b32 res = _GVkInit(app_name, 0, 0, 1, ext_cnt, exts, false, scratch_arena);
#endif
	if(!res) {
		log_err("Exiting, due to error during Vulkan initialisation...");
		return -1;
	}

	VkSurfaceKHR surface = createSurface(window);
	if(surface == VK_NULL_HANDLE) return -1;

	GVkDeviceOut dev = _GVkInitDevice(surface, false, &perm_arena, scratch_arena);
	if(dev.err) {
		log_err("Failed to initialise and create logical device. Exiting...");
		return -1;
	}

	GDevice device = dev.dev;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

	 _GVkCleanup(&device);
	glfwDestroyWindow(window);
	glfwTerminate();

  return 0;
}
