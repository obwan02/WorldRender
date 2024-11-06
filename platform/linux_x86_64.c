#include "platform.h"
#include "../core/core.h"
#include "../graphics/vulkan_api.h"

// for now, we rely on libc
#include <string.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

#include "../graphics/vulkan_api.c"
#include "../core/core.c"

#define PERM_ARENA_SIZE (4 * 1024 * 1024)
#define SCRATCH_ARENA_SIZE (1 * 1024 * 1024)

void wrld_memcpy(void* dest, const void* src, isize byte_count) {
	memcpy(dest, src, byte_count);
}

void wrld_memset(void* dest, u8 value, isize byte_count) {
	memset(dest, value, byte_count);
}

void wrld_memcmp(void *a, void *b, isize count, u32 mode) {
	u8 *bytes_a = a;
	u8 *bytes_b = b;
	u8 *end_a = bytes_a + count;

	// TODO: Add overflow assertions

	if(mode == MEMCMP_FAST) {
		for(; bytes_a != end_a; ++bytes_a, ++bytes_b) {
			if(*bytes_a != *bytes_b) return FALSE;
		}

		return TRUE;
	} else {
		// assume mode == MEMCMP_CONST_TIME)

		isize result = 0;
		for(; bytes_a != end_a; ++bytes_a, ++bytes_b) {
			result += *bytes_a ^ *bytes_b;
		}

		return result == 0;

	}

	hard_assert(false); // MODE UNDEFINED
}

void __break(void) {

	// Inspired by the scottt/debugbreak repo:
	// https://github.com/scottt/debugbreak
	
	// TODO: refactor to separate into x86_64 linux stuff
	// only

#ifdef _MSC_VER
	__debugbreak();
#else

#if __i386__ || __x86_64
	__asm__ volatile("int $0x03");
#elif __thumb__
	__asm__ volatile(".inst 0xde01");
#elif __arm__ && !__thumb__
	__asm__ volatile(".inst 0xe7f001f0");
#elif __aarch64__
	__asm__ volatile(".inst 0xd4200000");
#elif __powerpc__
	__asm__ volatile(".4byte 0x7d821008");
#elif __riscv
	__asm__ volatile(".4byte 0x00100073");
#else
#error Cannot generate debug break for target architecture
#endif
	
#endif // ifdef _MSC_VER

}

extern struct str app_name;
extern i8 app_start(void);

int main(int argc, const char *argv[]) {

	// TODO: Move away from using libc eventually
	struct arena scratch = arena_init(malloc(SCRATCH_ARENA_SIZE), SCRATCH_ARENA_SIZE);
	hard_assert(NULL != scratch.start);

	struct arena perm = arena_init(malloc(SCRATCH_ARENA_SIZE), SCRATCH_ARENA_SIZE);
	hard_assert(perm.start);

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
	b32 res = _gvk_init(app_name, 0, 0, 1, ext_cnt, exts, false, scratch);
	if(!res) {
		log_err("Exiting, due to error during Vulkan initialisation...");
		return -1;
	}

	VkSurfaceKHR surface = createSurface(window);
	if(surface == VK_NULL_HANDLE) return -1;

	GVKdevice = _GVkInitDevice(surface, false, &perm_arena, scratch_arena);
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
