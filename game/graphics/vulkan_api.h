/* Creation Date: 1/10/2023
 * Creator: obwan02
 * ========================
 *
 * Vulkan graphics header
 *
 */
#pragma once

#include "../core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef struct GDevice {
	VkDevice vkDevice;
	VkQueue vkGraphicsQueue, vkPresentQueue;

	// TODO: Don't know if this should be here????
	// I'll come back to this once I've become
	// more familiar with Vulkan
	VkSurfaceKHR vkSurface;
} GDevice;

struct GPresentParams {};
struct GShader {};
struct GBufferDescription {
	void *data;
	size_t size;
};

typedef struct GVkDeviceOut {
	GDevice dev;
	b32 err;
} GVkDeviceOut;

typedef b32(* _GVkCreateSurfaceFn)(VkInstance instance, void* userData, VkSurfaceKHR *out);

// Vulkan-only function to create vulkan instance. Should only be called from
// vulkan specific code - should not be called in generic rendering code.
//
// Parameters
// - appName: The name of the running application to provide to vulkan.
// - verMajor: The major version number of the application
// - verMinor: The minor version number of the application
// - verPatch: The patch version of the application
// - extCount: The number of extensions poi32ed to by the `extensions` ptr. Usually provided by GLFW
// - extensions: A ptr to the names of extensions to load. These extensions are loaded in addition to the
//							 normal extensions required by the rendering system. This shuold usually contain a list of native
//							 extensions that are required for specific platform i.e. for win32. Can be provided by GLFW.
// - 
b32 _GVkInit(Str app_name, i32 ver_maj, i32 ver_minor, i32 ver_patch, u32 platform_exts_count, const char **platform_exts, b32 portable_subset, Arena scatch);

// Initialise a graphics device. The best one will be chosen. It is not i32ended for multiple
// graphics devices to be created.
// TODO: Making this vulkan specific is a pain - however, kinda necessary. Evaluate
// this function signature in the future.
GVkDeviceOut _GVkInitDevice(_GVkCreateSurfaceFn createSurfaceFn, void* createSurfaceFnUserData, Arena scratch);

// Vulkan-only cleanup function to cleanup any left-over Vulkan resources. Honestly, probably doesn't need
// to be called as OS will clean up our resources for us (a lot faster than we can probs).
void _GVkCleanup(GDevice *device);

