/* Creation Date: 1/10/2023
 * Creator: obwan02
 * ========================
 *
 * Vulkan graphics header
 *
 */
#pragma once

#include "core/core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct gdevice {
	VkDevice vk_dev;
	VkQueue vk_graphics_q, vk_present_q;

	b32 vsync_lock;
	// TODO: Don't know if this should be here????
	// I'll come back to this once I've become
	// more familiar with Vulkan

	// TODO: Consider moving the surface and
	// swapchain + images to their own struct
	// called like 'GDisplayTarget'
	VkSurfaceKHR vk_surface;
	// Same here
	VkSwapchainKHR vk_swapchain;

	// @lifetime: permanent
	VkImage *vk_swapchain_imgs;
	// @lifetime: permanent
	VkImageView *vk_swapchain_img_views;
	u32 vk_swapchain_img_count;

	VkFormat vk_swapchain_fmt;
	VkExtent2D vk_swapchain_extent;
};

/* struct GPresentParams {}; */
/* struct GShader {}; */
/* struct GBufferDescription {}; */

// Vulkan-only function to create vulkan instance. Should only be called from
// vulkan specific code - should not be called in generic rendering code.
//
// Parameters
// - app_name: The name of the running application to provide to vulkan.
// - ver_major: The major version number of the application
// - ver_minor: The minor version number of the application
// - ver_patch: The patch version of the application
// - platform_exts_count: The number of extensions poi32ed to by the `extensions` ptr. 
// - platform_exts: A ptr to the names of extensions to load. These extensions are loaded in addition to the
//							 normal extensions required by the rendering system. This shuold usually contain a list of native
//							 extensions that are required for specific platform i.e. for win32. 
// - portable_flag: Sets the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR on VkInstanceCreateInfo
// - scratch: Used for temporary allocations during initialisation
b32 _gvk_init(struct str app_name, i32 ver_maj, i32 ver_minor, i32 ver_patch, u32 platform_exts_count, const char **platform_exts, b32 portable_flag, struct arena scratch);

// Initialise a graphics device. The best one will be chosen. It is not i32ended for multiple
// graphics devices to be created.
// TODO: Making this vulkan specific is a pain - however, kinda necessary. Evaluate
// this function signature in the future.
i32 _gvk_device_init(VkSurfaceKHR surface, b32 vsync_lock, struct gdevice *out, struct arena *perm, struct arena scratch);

// Vulkan-only cleanup function to cleanup any left-over Vulkan resources. Honestly, probably doesn't need
// to be called as OS will clean up our resources for us (a lot faster than we can probs).
void _gvk_device_cleanup(struct gdevice *device);

