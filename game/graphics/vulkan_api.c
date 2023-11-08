/* Creation Date: 1/10/2023
 * Creator: obwan02
 * ========================
 *
 * Vulkan graphics header
 *
 */
#include "vulkan_api.h"
#include "../util/assert.h"

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

static b32 checkAndPopulateDeviceExtensions(VkPhysicalDevice device, VkDeviceCreateInfo *createInfo);
static void checkAndPopulateInstanceExtensions(u32 extraExtsCount, const char **extraExtensions, b32 portableSubset, VkInstanceCreateInfo *out);
static b32 checkAndPopulateLayers(VkInstanceCreateInfo *out);
static PhysicalDeviceProps calcPhysicalDeviceProps(VkPhysicalDevice device, VkSurfaceKHR surface);

// TODO: Make work for multi-threaded code
static VkInstance _vkInstance;
static VkDebugUtilsMessengerEXT _vkDebugMessenger;

static const char *wantedInstanceExts[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};

static const char *wantedDeviceExts[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const char *wantedLayers[] = {
		"VK_LAYER_KHRONOS_validation",
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
_vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
                 VkDebugUtilsMessageTypeFlagsEXT msgType,
                 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                 void *pUserData) {
  log_errf("[VK VALIDATION LAYER] %s", pCallbackData->pMessage);
  return 0;
}

b32 _GVkInit(Str appName, i32 verMajor, i32 verMinor, i32 verPatch, u32 extraExtsCount, const char **extraExts, b32 portableSubset) {
							  
  ASSERT(appName.str != NULL);
  if (extraExtsCount)
    ASSERT(extraExts != NULL);

  VkDebugUtilsMessengerCreateInfoEXT debugCallbackCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = _vkDebugCallback,
      .pUserData = NULL};

	static u8 appNameBuffer[128];
	MutStr appNameStr = {
		.str = appNameBuffer,
		.len = 0,
		.cap = 128
	};
	EnsureCStr(appNameStr);

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = (const char *) appNameStr.str,
      .applicationVersion =
          VK_MAKE_API_VERSION(0, verMajor, verMinor, verPatch),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_API_VERSION(0, verMajor, verMinor, verPatch),
      .apiVersion = VK_API_VERSION_1_0,
  };


  VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .pNext =
          &debugCallbackCreateInfo // Have debugging before instance is created
  };

	checkAndPopulateInstanceExtensions(extraExtsCount, extraExts, portableSubset, &createInfo);
	/* if(res != RESULT_SUCCESS) { */
	/* 	log_err("Could not load all instance extensions, cannot initialise vulkan"); */
	/* 	return res; */
	/* } */

	// Ignore any errors relating to not loading
	// layers :)
	b32 _ = checkAndPopulateLayers(&createInfo);

	if(portableSubset) {
		createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
	
  // Create the instance
  VkResult result = vkCreateInstance(&createInfo, NULL, &_vkInstance);
  if (result != VK_SUCCESS) {
    log_err("Failed to create vkInstance");
    return false;
  }

  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          _vkInstance, "vkCreatDebugUtilsMessengerEXT");
  if (vkCreateDebugUtilsMessengerEXT) {
    if (vkCreateDebugUtilsMessengerEXT(_vkInstance, &debugCallbackCreateInfo,
                                       NULL,
                                       &_vkDebugMessenger) != VK_SUCCESS) {
      log_err("Failed to register vulkan debug callback");
      return false;
    }
  }

  return true;
}

GVkDeviceOut _GVkInitDevice(_GVkCreateSurfaceFn createSurfaceFn, void* createSurfaceFnUserData)  {

	// Create the surface that the device will draw to
	VkSurfaceKHR surface;
	b32 res = createSurfaceFn(_vkInstance, createSurfaceFnUserData, &surface);
	if(!res) {
		log_err("Failed to create vulkan surface from user callback");
	}

	uint32_t count = 0;
	static VkPhysicalDevice allDevices[128];
	
	vkEnumeratePhysicalDevices(_vkInstance, &count, NULL);
	ASSERT(count <= 128);
	vkEnumeratePhysicalDevices(_vkInstance, &count, allDevices);

	// Pick the best device that is suitable for the job
	// TODO: Allow user to override decision made by us
	
	int maxSuitability = -100;
	VkPhysicalDevice chosenDevice = VK_NULL_HANDLE;
	PhysicalDeviceProps chosenDeviceProps;

	for(int i = 0; i < count; i++) {

		PhysicalDeviceProps props = calcPhysicalDeviceProps(allDevices[i], surface);

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
		.enabledLayerCount = COUNT_OF(wantedLayers),
		.ppEnabledLayerNames = wantedLayers
	};

	res = checkAndPopulateDeviceExtensions(chosenDevice, &createInfo);
	// This shouldn't fail, as no devices should pass the minimum device requirements
	// and fail this assertion
	ASSERT(res);

	VkDevice logicalDevice;
	if(vkCreateDevice(chosenDevice, &createInfo, NULL, &logicalDevice) != VK_SUCCESS) {
		log_err("Failed to create logical vulkan device");
		return (GVkDeviceOut) { .err = true };
	}

	VkQueue graphicsQueue;
	vkGetDeviceQueue(logicalDevice, chosenDeviceProps.graphicsFamilyIndex, 0, &graphicsQueue);

	// PICKUP: Calling create surface callback, and creating VkSurfaceKHR 
	// as well as VkSwapChainKHR :))

	return (GVkDeviceOut) { {
		.vkDevice = logicalDevice,
		.vkGraphicsQueue = graphicsQueue,
		.vkSurface = surface,
	} };
}

static PhysicalDeviceProps calcPhysicalDeviceProps(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

	if(checkAndPopulateDeviceExtensions(device, NULL)) {
		result.hasRequiredExtensions = true;
	} else {
		result.meetsMinsRequirements = false;
	}

	// No ranking system yet: TODO
	result.score = 100;
	return result;
}

static void checkAndPopulateInstanceExtensions(u32 extraExtsCount, const char **extraExtensions, b32 portableSubset, VkInstanceCreateInfo *out) {
	// TODO: Convert to memory arena
  static const char *extsArr[128];
  ASSERT(COUNT_OF(wantedInstanceExts) + extraExtsCount <= COUNT_OF(extsArr));
  Copy(extsArr, wantedInstanceExts, sizeof(wantedInstanceExts));
	Copy(extsArr + COUNT_OF(wantedInstanceExts), extraExtensions, extraExtsCount * sizeof(const char *));

	if(portableSubset) {
		ASSERT(COUNT_OF(wantedInstanceExts) + extraExtsCount + 1 <= COUNT_OF(extsArr));
		extsArr[COUNT_OF(wantedInstanceExts) + extraExtsCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
		out->enabledExtensionCount = extraExtsCount + COUNT_OF(wantedInstanceExts) + 1;
	} else {
		out->enabledExtensionCount = extraExtsCount + COUNT_OF(wantedInstanceExts);
	}

  out->ppEnabledExtensionNames = extsArr;

	// TODO: Check for extension availability
}

static b32 checkAndPopulateDeviceExtensions(VkPhysicalDevice device, VkDeviceCreateInfo *createInfo) {

	u32 allExtensionsCount;
	static VkExtensionProperties allExtensionProps[256];

	vkEnumerateDeviceExtensionProperties(device, NULL, &allExtensionsCount, NULL);
	ASSERT(allExtensionsCount <= COUNT_OF(allExtensionProps));
	vkEnumerateDeviceExtensionProperties(device, NULL, &allExtensionsCount, allExtensionProps);

	b32 foundAllExts = true;
	for(size_t i = 0; i < COUNT_OF(wantedDeviceExts); i++) {
		b32 found = false;
		for(size_t j = 0; j < allExtensionsCount; j++) {
			// PICKUP: StrEq not working (I don't think)
			Str compare = StrFromCStr(allExtensionProps[j].extensionName);
			Str wanted = StrFromCStr(wantedDeviceExts[i]);
			if(StrEq(compare, wanted))
				found = true;
		}

		foundAllExts = foundAllExts && found;

		// TODO: Return array of missing extensions
		if(!found)
			log_warnf("WARN: Found a vulkan device that doesn't support extension '%s'", wantedDeviceExts[i]);
	}

	if(createInfo) {
		createInfo->ppEnabledExtensionNames = wantedDeviceExts;
		createInfo->enabledExtensionCount = COUNT_OF(wantedDeviceExts);
	}

	return foundAllExts;
}

static b32 checkAndPopulateLayers(VkInstanceCreateInfo *out) {
  u32 layerCount;
  static VkLayerProperties availableLayers[32];

	// Look at all the possible layers we can load,
	// to see if we can load our desired layers
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);
  ASSERT(layerCount <= 32);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	b32 foundAll = true;
  // Search for the layers we want
  // O(n^2) Tasty!
  for (size_t i = 0; i < COUNT_OF(wantedLayers); i++) {
    b32 found = false;

    for (size_t j = 0; j < layerCount; j++) {
			Str compare = StrFromCStr(availableLayers[j].layerName);
			Str wanted = StrFromCStr(wantedLayers[i]);

      if (StrEq(compare, wanted)) {
        found = true;
        break;
      }
    }

    if (!found) {
      log_warnf("Layer required, but doesn't exist: %s", wantedLayers[i]);
			foundAll = false;
    }
  }

  out->enabledLayerCount = COUNT_OF(wantedLayers);
  out->ppEnabledLayerNames = wantedLayers;
	return true;
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
          _vkInstance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != NULL) {
    func(_vkInstance, _vkDebugMessenger, NULL);
  }
  vkDestroyInstance(_vkInstance, NULL);
}
