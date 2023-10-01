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

namespace g {

	// Vulkan-only function to create vulkan instance. Should only be called from
	// vulkan specific code - should not be called in generic rendering code
	Error _vkInitInstance(const char *appName, int verMajor, int verMinor, int verPatch, uint32_t extCount, const char **extensions);

} // namespace g
