/* Creation Date: 1/10/2023
 * Creator: obwan02
 * ========================
 *
 * Vulkan graphics header
 *
 */
#include "vulkan_api.h"
#include <vulkan/vulkan_core.h>

#define NULL_QUEUE_FAM_INDEX UINT32_MAX

typedef struct PhysicalDeviceProps {
	i32 score;
	
	u32 graphicsFamilyIndex;
	u32 presentIndex;

	b32 meetsMinsRequirements;
	b32 hasRequiredExtensions;
} PhysicalDeviceProps;

typedef struct PopDeviceExtsOut {
		
} PopDeviceExtsOut;

static b32 checkDeviceExtensions(VkPhysicalDevice, Arena);
static b32 checkLayers(Arena);
static PhysicalDeviceProps calcPhysicalDeviceProps(VkPhysicalDevice, VkSurfaceKHR, Arena scratch);

// TODO: Make work for multi-threaded code
static VkInstance _vk_instance;
static VkDebugUtilsMessengerEXT _vk_dbg_messenger;

static const char *wanted_instance_exts[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};

static const char *wanted_device_exts[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const char *wanted_layers[] = {
		"VK_LAYER_KHRONOS_validation",
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
_vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msg_sev,
                 VkDebugUtilsMessageTypeFlagsEXT msg_type,
                 const VkDebugUtilsMessengerCallbackDataEXT *pcallback_data,
                 void *puser_data) {
  log_errf("[VK VALIDATION LAYER] %s", pcallback_data->pMessage);
  return 0;
}

b32 _GVkInit(Str app_name, i32 ver_maj, i32 ver_minor, i32 ver_patch, u32 platform_exts_count, const char **platform_exts, b32 portable_subset, Arena scratch) {
							  
  ASSERT(app_name.str != NULL);
  if (platform_exts_count)
    ASSERT(platform_exts != NULL);

  VkDebugUtilsMessengerCreateInfoEXT dbg_callback_create_inf = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = _vkDebugCallback,
      .pUserData = NULL};

	// Ensure the app name is a null terminated string.
	MutStr app_name_str = {
		.str = New(&scratch, u8, app_name.len + 1),
		.len = 0,
		.cap = app_name.len + 1
	};
	StrCpy(app_name, app_name_str);
	EnsureCStr(app_name_str);

  VkApplicationInfo appinf = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = (const char *) app_name_str.str,
      .applicationVersion =
          VK_MAKE_API_VERSION(0, ver_maj, ver_minor, ver_patch),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_API_VERSION(0, ver_maj, ver_minor, ver_patch),
      .apiVersion = VK_API_VERSION_1_0,
  };


  VkInstanceCreateInfo inst_createinfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appinf,
      .pNext =
          &dbg_callback_create_inf // Have debugging before instance is created
  };

	isize inst_ext_cnt = COUNT_OF(wanted_instance_exts) + platform_exts_count + (portable_subset ? 1 : 0);
	const char **final_arr = New(&scratch, const char *, inst_ext_cnt);

  MemCopy(final_arr, wanted_instance_exts, sizeof(wanted_instance_exts));
	MemCopy(final_arr + COUNT_OF(wanted_instance_exts), platform_exts, platform_exts_count * sizeof(const char *));

	if(portable_subset) {
		final_arr[COUNT_OF(wanted_instance_exts) + platform_exts_count] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
	}

	inst_createinfo.enabledExtensionCount = inst_ext_cnt;
  inst_createinfo.ppEnabledExtensionNames = final_arr;

	// Ignore any errors relating to not loading
	// layers :)
	(void) checkLayers(scratch);
	inst_createinfo.ppEnabledLayerNames = wanted_layers;
	inst_createinfo.enabledLayerCount = COUNT_OF(wanted_layers);

	if(portable_subset) {
		inst_createinfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
	
  // Create the instance
  VkResult result = vkCreateInstance(&inst_createinfo, NULL, &_vk_instance);
  if (result != VK_SUCCESS) {
    log_err("Failed to create vkInstance");
    return false;
  }

  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          _vk_instance, "vkCreatDebugUtilsMessengerEXT");
  if (vkCreateDebugUtilsMessengerEXT) {
    if (vkCreateDebugUtilsMessengerEXT(_vk_instance, &dbg_callback_create_inf,
                                       NULL,
                                       &_vk_dbg_messenger) != VK_SUCCESS) {
      log_err("Failed to register vulkan debug callback");
      return false;
    }
  }

  return true;
}

GVkDeviceOut _GVkInitDevice(_GVkCreateSurfaceFn createSurfaceFn, void* createSurfaceFnUserData, Arena scratch)  {

	// Create the surface that the device will draw to
	VkSurfaceKHR surface;
	b32 res = createSurfaceFn(_vk_instance, createSurfaceFnUserData, &surface);
	if(!res) {
		log_err("Failed to create vulkan surface from user callback");
	}

	uint32_t count = 0;
	static VkPhysicalDevice allDevices[128];
	
	vkEnumeratePhysicalDevices(_vk_instance, &count, NULL);
	ASSERT(count <= 128);
	vkEnumeratePhysicalDevices(_vk_instance, &count, allDevices);

	// Pick the best device that is suitable for the job
	// TODO: Allow user to override decision made by us
	
	int maxSuitability = -100;
	VkPhysicalDevice chosenDevice = VK_NULL_HANDLE;
	PhysicalDeviceProps chosenDeviceProps;

	for(int i = 0; i < count; i++) {

		PhysicalDeviceProps props = calcPhysicalDeviceProps(allDevices[i], surface, scratch);

		if(props.meetsMinsRequirements && props.score > maxSuitability) {
			chosenDevice = allDevices[i];
			chosenDeviceProps = props;
		}
	}

	if(chosenDevice == VK_NULL_HANDLE) {
		// TODO: Maybe remove logging and replace with a return param indicating
		// how many physical devices couldn't be found
		log_err("Couldn't find any physical devices that meet minimum requirements");
		return (GVkDeviceOut) { .err = true, };
	}

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = chosenDeviceProps.graphicsFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority,
	};

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pQueueCreateInfos = &queueCreateInfo,
		.queueCreateInfoCount = 1,
		.pEnabledFeatures = &deviceFeatures,

		// TODO: Remove dependency on global variable, so 
		// it is more future / bullet proof
		.enabledLayerCount = COUNT_OF(wanted_layers),
		.ppEnabledLayerNames = wanted_layers
	};

	// This shouldn't fail, as no devices should pass the minimum device requirements
	// and fail this assertion
	ASSERT(checkDeviceExtensions(chosenDevice, scratch));
	createInfo.ppEnabledExtensionNames = wanted_device_exts;
	createInfo.enabledLayerCount = COUNT_OF(wanted_device_exts);

	VkDevice logicalDevice;
	if(vkCreateDevice(chosenDevice, &createInfo, NULL, &logicalDevice) != VK_SUCCESS) {
		log_err("Failed to create logical vulkan device");
		return (GVkDeviceOut) { .err = true };
	}

	VkQueue graphicsQueue;
	vkGetDeviceQueue(logicalDevice, chosenDeviceProps.graphicsFamilyIndex, 0, &graphicsQueue);

	// PICKUP: Calling create surface callback, and creating VkSurfaceKHR 
	// as well as VkSwapChainKHR :))

	return (GVkDeviceOut) { 
		.dev = {
		  .vkDevice = logicalDevice,
		  .vkGraphicsQueue = graphicsQueue,
		  .vkSurface = surface,
	}, .err = false};
}

static PhysicalDeviceProps calcPhysicalDeviceProps(VkPhysicalDevice device, VkSurfaceKHR surface, Arena scratch) {
	VkPhysicalDeviceProperties deviceProps;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(device, &deviceProps);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	PhysicalDeviceProps result = {
		.hasRequiredExtensions = false,
		.meetsMinsRequirements = false,
		.graphicsFamilyIndex = NULL_QUEUE_FAM_INDEX,
		.presentIndex = NULL_QUEUE_FAM_INDEX,
		.score = -1
	};
	 

	result.meetsMinsRequirements = deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
														 		 deviceFeatures.geometryShader;

	// *** Check graphics family queues ***
	uint32_t queueFamilyCount = 0;
	static VkQueueFamilyProperties queueFamilies[128];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	ASSERT(queueFamilyCount <= COUNT_OF(queueFamilies));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for(size_t i = 0; i < queueFamilyCount; i++) {
		if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			result.graphicsFamilyIndex = i;
		}

		VkBool32 queueFamilySupportsPresent = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &queueFamilySupportsPresent);

		if(queueFamilySupportsPresent) {
			if(result.presentIndex == NULL_QUEUE_FAM_INDEX) 
				result.presentIndex = i;

			// Prioritise keeping the present and graphics queue the same
			// TODO: Check if this is good or bad
			else if(result.presentIndex == result.graphicsFamilyIndex)
				continue;

			result.presentIndex = i;
		}
	}

	if(result.graphicsFamilyIndex == NULL_QUEUE_FAM_INDEX || result.presentIndex == NULL_QUEUE_FAM_INDEX) {
		result.meetsMinsRequirements = false;
	}

	if(checkDeviceExtensions(device, scratch)) {
		result.hasRequiredExtensions = true;
	} else {
		result.meetsMinsRequirements = false;
	}

	// No ranking system yet: TODO
	result.score = 100;
	return result;
}

static b32 checkDeviceExtensions(VkPhysicalDevice device, Arena scratch) {

	u32 all_exts_cnt;
	VkExtensionProperties *all_exts;

	vkEnumerateDeviceExtensionProperties(device, NULL, &all_exts_cnt, NULL);
	all_exts = New(&scratch, VkExtensionProperties, all_exts_cnt);
	vkEnumerateDeviceExtensionProperties(device, NULL, &all_exts_cnt, all_exts);

	b32 found_all = true;
	for(size_t i = 0; i < COUNT_OF(wanted_device_exts); i++) {
		b32 found = false;
		for(size_t j = 0; j < all_exts_cnt; j++) {
			// PICKUP: StrEq not working (I don't think)
			Str compare = StrFromCStr(all_exts[j].extensionName);
			Str wanted = StrFromCStr(wanted_device_exts[i]);
			if(StrEq(compare, wanted))
				found = true;
		}

		found_all = found_all && found;

		// TODO: Return array of missing extensions
		if(!found)
			log_warnf("WARN: Found a vulkan device that doesn't support extension '%s'", wanted_device_exts[i]);
	}

	return found_all;
}

static b32 checkLayers(Arena scratch) {
  u32 layer_cnt;
	VkLayerProperties *all_layers;

	// Look at all the possible layers we can load,
	// to see if we can load our desired layers
  vkEnumerateInstanceLayerProperties(&layer_cnt, NULL);
	all_layers = New(&scratch, VkLayerProperties, layer_cnt);
  vkEnumerateInstanceLayerProperties(&layer_cnt, all_layers);

	b32 found_all = true;
  // Search for the layers we want
  // O(n^2) Tasty!
  for (size_t i = 0; i < COUNT_OF(wanted_layers); i++) {
    b32 found = false;

    for (size_t j = 0; j < layer_cnt; j++) {
			Str compare = StrFromCStr(all_layers[j].layerName);
			Str wanted = StrFromCStr(wanted_layers[i]);

      if (StrEq(compare, wanted)) {
        found = true;
        break;
      }
    }

    if (!found) {
      log_warnf("Layer required, but doesn't exist: %s", wanted_layers[i]);
			found_all = false;
    }
  }

	return found_all;
}

// TODO: Get rid of device from here
// Maybe have some kind of automatic cleanup for vulkan objects??
// Or maybe just store the device globally if we assume we only have one
// device
void _GVkCleanup(GDevice *device) {
	if(device != NULL) {
		vkDestroyDevice(device->vkDevice, NULL);
	}

  PFN_vkDestroyDebugUtilsMessengerEXT func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          _vk_instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != NULL) {
    func(_vk_instance, _vk_dbg_messenger, NULL);
  }
  vkDestroyInstance(_vk_instance, NULL);
}
