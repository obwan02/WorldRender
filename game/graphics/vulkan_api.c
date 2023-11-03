/* Creation Date: 1/10/2023
 * Creator: obwan02
 * ========================
 *
 * Vulkan graphics header
 *
 */
#include "vulkan_api.h"
#include "../util/assert.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

typedef struct PhysicalDeviceProps {
	int score;
	
	uint32_t graphicsFamilyIndex;

	bool meetsMinsRequirements;
} PhysicalDeviceProps;

static void populateVulkanExtensions(uint32_t extraExtsCount, const char **extraExtensions, VkInstanceCreateInfo *out);
static Result populateVulkanLayers(VkInstanceCreateInfo *out);
static PhysicalDeviceProps calcPhysicalDeviceProps(VkPhysicalDevice device);

// TODO: Make work for multi-threaded code
static VkInstance _vkInstance;
static VkDebugUtilsMessengerEXT _vkDebugMessenger;

static const char *wantedExts[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
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

Result _GVkInit(const char *appName, int verMajor, int verMinor, int verPatch, uint32_t extraExtsCount, const char **extraExts) {
							  
  ASSERT(appName != NULL);
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

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = appName,
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

	populateVulkanExtensions(extraExtsCount, extraExts, &createInfo);
	populateVulkanLayers(&createInfo);

  // Create the instance
  VkResult result = vkCreateInstance(&createInfo, NULL, &_vkInstance);
  if (result != VK_SUCCESS) {
    log_err("Failed to create vkInstance");
    return RESULT_GENERIC_ERR;
  }

  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          _vkInstance, "vkCreatDebugUtilsMessengerEXT");
  if (vkCreateDebugUtilsMessengerEXT) {
    if (vkCreateDebugUtilsMessengerEXT(_vkInstance, &debugCallbackCreateInfo,
                                       NULL,
                                       &_vkDebugMessenger) != VK_SUCCESS) {
      log_err("Failed to register vulkan debug callback");
      return RESULT_GENERIC_ERR;
    }
  }

  return RESULT_SUCCESS;
}

Result _GVkInitDevice(GDevice *out, _GVkCreateSurfaceFn createSurfaceFn, void* createSurfaceFnUserData)  {
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

		PhysicalDeviceProps props = calcPhysicalDeviceProps(allDevices[i]);

		if(props.meetsMinsRequirements && props.score > maxSuitability) {
			chosenDevice = allDevices[i];
			chosenDeviceProps = props;
		}
	}

	if(chosenDevice == VK_NULL_HANDLE) {
		log_err("Vulkan: Couldn't find any physical devices that meet minimum requirements");
		return RESULT_GENERIC_ERR;
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

		.enabledExtensionCount = 0,

		// TODO: Remove dependency on global variable, so 
		// it is more future / bullet proof
		.enabledLayerCount = COUNT_OF(wantedLayers),
		.ppEnabledLayerNames = wantedLayers
	};

	VkDevice logicalDevice;
	if(vkCreateDevice(chosenDevice, &createInfo, NULL, &logicalDevice) != VK_SUCCESS) {
		log_err("Failed to create logical vulkan device");
		return RESULT_GENERIC_ERR;
	}

	VkQueue graphicsQueue;
	vkGetDeviceQueue(logicalDevice, chosenDeviceProps.graphicsFamilyIndex, 0, &graphicsQueue);

	// PICKUP: Calling create surface callback, and creating VkSurfaceKHR 
	// as well as VkSwapChainKHR :))

	*out = (GDevice) {
		.vkDevice = logicalDevice,
		.vkGraphicsQueue = graphicsQueue
	};

	return RESULT_SUCCESS;
}

static PhysicalDeviceProps calcPhysicalDeviceProps(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProps;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(device, &deviceProps);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	PhysicalDeviceProps result = {
		.meetsMinsRequirements = false,
		.graphicsFamilyIndex = UINT32_MAX,
		.score = -1
	};
	 
	result.meetsMinsRequirements = deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
														 		 deviceFeatures.geometryShader;

	// *** Check graphics family queues ***
	result.graphicsFamilyIndex = UINT32_MAX;
	uint32_t queueFamilyCount = 0;
	static VkQueueFamilyProperties queueFamilies[128];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	ASSERT(queueFamilyCount <= COUNT_OF(queueFamilies));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for(size_t i = 0; i < queueFamilyCount; i++) {
		if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			result.graphicsFamilyIndex = i;
		}
	}

	if(result.graphicsFamilyIndex == UINT32_MAX) {
		result.meetsMinsRequirements = false;
	}


	// No ranking system yet: TODO
	result.score = 100;
	return result;
}

static void populateVulkanExtensions(uint32_t extraExtsCount, const char **extraExtensions, VkInstanceCreateInfo *out) {
  static const char *extsArr[128];
  ASSERT(COUNT_OF(wantedExts) + extraExtsCount <= COUNT_OF(extsArr));
  memcpy(extsArr, wantedExts, sizeof(wantedExts));
  memcpy(extsArr + COUNT_OF(wantedExts), extraExtensions, extraExtsCount * sizeof(const char *));

  out->enabledExtensionCount = extraExtsCount + COUNT_OF(wantedExts);
  out->ppEnabledExtensionNames = extsArr;
}

static Result populateVulkanLayers(VkInstanceCreateInfo *out) {
  uint32_t layerCount;
  static VkLayerProperties availableLayers[32];

	// Look at all the possible layers we can load,
	// to see if we can load our desired layers
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);
  ASSERT(layerCount <= 32);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

  // Search for the layers we want
  // O(n^2) Tasty!
  for (size_t i = 0; i < COUNT_OF(wantedLayers); i++) {
    bool found = false;

    for (size_t j = 0; j < layerCount; j++) {
      if (strcmp(wantedLayers[i], availableLayers[j].layerName) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      log_errf("Layer required, but doesn't exist: %s", wantedLayers[i]);
      return RESULT_GENERIC_ERR;
    }
  }

  out->enabledLayerCount = COUNT_OF(wantedLayers);
  out->ppEnabledLayerNames = wantedLayers;
	return RESULT_SUCCESS;
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
