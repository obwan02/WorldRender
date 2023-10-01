/* Creation Date: 1/10/2023
 * Creator: obwan02
 * ========================
 *
 * Vulkan graphics header
 *
 */
#include "vulkan_api.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

class VulkanError : public ErrorBase {
public:
	VulkanError(VkResult result) : vkResult(result) {}

	void print() {
		printf("Vulkan Error Occured: %s\r\n", string_VkResult(this->vkResult));
	}

	VkResult vkResult;
};

namespace g {
	// TODO: Make work for multi-threaded code
	static VkInstance _vkInstance;

	// Vulkan specific
	Error _vkInitInstance(const char *appName, int verMajor, int verMinor, int verPatch, uint32_t extCount, const char **extensions) {
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, verMajor, verMinor, verPatch);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, verMajor, verMinor, verPatch);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		if(VkResult result = vkCreateInstance(&createInfo, nullptr, &_vkInstance); result != VK_SUCCESS) {
			return AllocError(VulkanError(result));
		}

		return NO_ERROR;
	}
}
