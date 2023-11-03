#include <X11/X.h>
#include <stdio.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_X11_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

#include "game/core.h"
#include "game/graphics/vulkan_api.h"

// #include "game/game.cpp"
#include "game/graphics/vulkan_api.c"
#include "game/util/clang_assert.c"

static const char* AppName = "World-Scale Renderer";

typedef struct CreateSurfaceData {
	Window xlibWin;
} CreateSurfaceData;

//  X11/Win32/Coca    Vulkan Instance
//   |                  v----/
//   V                  |
//  Surface Create Fn   |
//   |                  |
//   V                  /
//  _GVkInit        <--/
static Result createSurface(VkInstance instance, VkSurfaceKHR *out, void *data) {
	CreateSurfaceData *userData = data;

	Display *disp = glfwGetX11Display();
	VkXlibSurfaceCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.window = userData->xlibWin,
		.dpy = disp,
	};

	// PICKUP: Need access to vulkan instance to create surface.
	// Kinda hard to move this code to the graphics API as it's platform
	// specific.
	VkResult res = vkCreateXlibSurfaceKHR(instance, &createInfo, NULL, out);
	return (res == VK_SUCCESS) ? RESULT_SUCCESS : RESULT_GENERIC_ERR;
}

int main(int argc, const char *argv[]) {

  if (!glfwInit()) {
    puts("Failed to initialise GLFW. Exiting ...");
    return -1;
  }

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
		.xlibWin = glfwGetX11Window(window)
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
