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


typedef struct SwapchainProps {
	b32 meets_min_reqs;
	
	VkSurfaceFormatKHR fmt;
	VkPresentModeKHR mode;
	VkExtent2D extent;
	VkSurfaceTransformFlagBitsKHR pre_transform;
	u32 img_cnt;
} SwapchainProps;

typedef struct PDeviceProps {
	i32 score;
	
	u32 graphics_qfam_idx;
	u32 present_qfam_idx;

	SwapchainProps swapchain_props;

	b32 meets_min_reqs;
	b32 has_wanted_extensions;
} PDeviceProps;

static b32 checkDeviceExtensions(VkPhysicalDevice, const char**, u32, Arena);
static b32 checkLayers(const char **, u32, Arena);
static SwapchainProps querySwapchain(VkPhysicalDevice, VkSurfaceKHR, b32, Arena);
static PDeviceProps queryPDeviceProps(VkPhysicalDevice, VkSurfaceKHR, b32, Arena);

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
    /* VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001, */
    /* VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010, */
    /* VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100, */
    /* VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000, */
    /* VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF */
	if((msg_sev & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) || (msg_sev & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
		log_errf(":: [VK VALIDATION LAYER] %s", pcallback_data->pMessage);

	else if(msg_sev & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		log_warnf(":: [VK VALIDATION LAYER] %s", pcallback_data->pMessage);
	
	else 
		log_errf(":: [VK VALIDATION LAYER] %s", pcallback_data->pMessage);
	

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

	log_dbg("Vulkan Instance Layers Requested: ");
	for(isize i = 0; i < inst_ext_cnt; i++) {
		log_dbgf("\t- %s", final_arr[i]);
	}

	inst_createinfo.enabledExtensionCount = inst_ext_cnt;
  inst_createinfo.ppEnabledExtensionNames = final_arr;

	// Ignore any errors relating to not loading
	// layers :)
	(void) checkLayers(wanted_layers, COUNT_OF(wanted_layers), scratch);
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

GVkDeviceOut _GVkInitDevice(_GVkCreateSurfaceFn createSurfaceFn, void* createSurfaceFnUserData, b32 vsync_lock, Arena scratch)  {

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
	
	int max_suit = -100;
	VkPhysicalDevice chosen_dev = VK_NULL_HANDLE;
	PDeviceProps chosen_dev_props;

	for(int i = 0; i < count; i++) {

		PDeviceProps props = queryPDeviceProps(allDevices[i], surface, vsync_lock, scratch);

		if(props.meets_min_reqs && props.score > max_suit) {
			chosen_dev = allDevices[i];
			chosen_dev_props = props;
		}
	}

	if(chosen_dev == VK_NULL_HANDLE) {
		// TODO: Maybe remove logging and replace with a return param indicating
		// how many physical devices couldn't be found
		log_err("Couldn't find any physical devices that meet minimum requirements");
		return (GVkDeviceOut) { .err = true, };
	}

	log_dbgf("Graphics Queue Family Index: %u", chosen_dev_props.graphics_qfam_idx);
	log_dbgf("Present Queue Family Index: %u", chosen_dev_props.present_qfam_idx);

	float q_priority = 1.0f;
	u32 all_qfam_idxs[] = {chosen_dev_props.graphics_qfam_idx, chosen_dev_props.present_qfam_idx};

	// Not using whole array, have to keep track of # items
	VkDeviceQueueCreateInfo q_create_infos[COUNT_OF(all_qfam_idxs)];
	isize q_create_infos_cnt = 0;

	for(isize i = 0; i < COUNT_OF(all_qfam_idxs); i++) {
		b32 dup = false;
		for(isize j = 0; j < q_create_infos_cnt; j++) {
			if(all_qfam_idxs[i] == q_create_infos[j].queueFamilyIndex) dup = true;
		}

		if(dup) continue;

		q_create_infos[q_create_infos_cnt++] = (VkDeviceQueueCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = all_qfam_idxs[i],
			.queueCount = 1,
			.pQueuePriorities = &q_priority,
		};
	}

	log_dbgf("Unique Queue Family Indices: %ld", q_create_infos_cnt);

	VkPhysicalDeviceFeatures deviceFeatures = {0};

	VkDeviceCreateInfo create_inf = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pQueueCreateInfos = q_create_infos,
		.queueCreateInfoCount = q_create_infos_cnt,
		.pEnabledFeatures = &deviceFeatures,

		// TODO: Remove dependency on global variable, so 
		// it is more future / bullet proof
		.enabledLayerCount = COUNT_OF(wanted_layers),
		.ppEnabledLayerNames = wanted_layers
	};

	// Sanity check. These shouldn't fail, as no devices should pass 
	// the minimum device requirements and fail these assertions.
	ASSERT(checkDeviceExtensions(chosen_dev, wanted_device_exts, COUNT_OF(wanted_device_exts), scratch));
	ASSERT(chosen_dev_props.swapchain_props.meets_min_reqs);

	create_inf.ppEnabledExtensionNames = wanted_device_exts;
	create_inf.enabledLayerCount = COUNT_OF(wanted_device_exts);

	VkDevice logical_dev;
	if(vkCreateDevice(chosen_dev, &create_inf, NULL, &logical_dev) != VK_SUCCESS) {
		log_err("Failed to create logical vulkan device");
		return (GVkDeviceOut) { .err = true };
	}

	VkQueue graphics_q, present_q;
	vkGetDeviceQueue(logical_dev, chosen_dev_props.graphics_qfam_idx, 0, &graphics_q);
	vkGetDeviceQueue(logical_dev, chosen_dev_props.present_qfam_idx, 0, &present_q);

	// PICKUP: Creating VkSwapChainKHR :))
	SwapchainProps *sc_props = &chosen_dev_props.swapchain_props;

	b32 qfam_same = chosen_dev_props.present_qfam_idx == chosen_dev_props.graphics_qfam_idx;
	u32 qfam_idxs[] = {chosen_dev_props.present_qfam_idx, chosen_dev_props.graphics_qfam_idx};

	VkSwapchainCreateInfoKHR sc_create_inf = {
			.minImageCount = sc_props->img_cnt,
			.imageFormat = sc_props->fmt.format,
			.imageColorSpace = sc_props->fmt.colorSpace,
			.imageExtent = sc_props->extent,
			.imageArrayLayers = 1, // Apparently this is != 1 if you're doing like VR 
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			// TODO: Stuff and things (ownership of stuff)
			.imageSharingMode = qfam_same ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
			.queueFamilyIndexCount = qfam_same ? 0 : COUNT_OF(qfam_idxs),
			.pQueueFamilyIndices = qfam_same ? NULL : qfam_idxs,
			.preTransform = sc_props->pre_transform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = sc_props->mode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
	};

	VkSwapchainKHR swapchain;
	if(vkCreateSwapchainKHR(logical_dev, &sc_create_inf, NULL, &swapchain) != VK_SUCCESS) {
		log_err("Failed to create vulkan swapchain");
		return (GVkDeviceOut) { .err = true };
	}

	return (GVkDeviceOut) { 
		.dev = {
		  .vk_dev = logical_dev,
		  .vk_graphics_q = graphics_q,
			.vk_present_q = present_q,
		  .vk_surf = surface,
			.vk_swapchain = swapchain,
	}, .err = false};
}

static PDeviceProps queryPDeviceProps(VkPhysicalDevice device, VkSurfaceKHR surface, b32 vsync_lock, Arena scratch) {
	VkPhysicalDeviceProperties dev_props;
	VkPhysicalDeviceFeatures dev_feats;

	vkGetPhysicalDeviceProperties(device, &dev_props);
	vkGetPhysicalDeviceFeatures(device, &dev_feats);

	PDeviceProps result = {
		.has_wanted_extensions = false,
		.meets_min_reqs = false,
		.graphics_qfam_idx = NULL_QUEUE_FAM_INDEX,
		.present_qfam_idx = NULL_QUEUE_FAM_INDEX,
		.score = -1
	};

	// TODO: Change q_fams to using Arena?
	// *** Check graphics family queues ***
	uint32_t q_fam_cnt = 0;
	static VkQueueFamilyProperties q_fams[128];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &q_fam_cnt, NULL);
	ASSERT(q_fam_cnt <= COUNT_OF(q_fams));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &q_fam_cnt, q_fams);

	for(size_t i = 0; i < q_fam_cnt; i++) {
		if(q_fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			result.graphics_qfam_idx = i;
		}

		VkBool32 queueFamilySupportsPresent = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &queueFamilySupportsPresent);

		if(queueFamilySupportsPresent) {
			if(result.present_qfam_idx == NULL_QUEUE_FAM_INDEX) 
				result.present_qfam_idx = i;

			// Prioritise keeping the present and graphics queue the same
			// TODO: Check if this is good or bad
			else if(result.present_qfam_idx == result.graphics_qfam_idx)
				continue;

			result.present_qfam_idx = i;
		}
	}

	result.has_wanted_extensions = checkDeviceExtensions(device, wanted_device_exts, COUNT_OF(wanted_device_exts), scratch);
	result.swapchain_props = querySwapchain(device, surface, vsync_lock, scratch);

	// TODO: Re-enable for non macOS. M2 doesn't seem to support geometryShaders???
	// result.meetsMinsRequirements = deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	static i32 pdevice_num = 0;
	log_dbgf("Physical Device %d\n - present_qfam_idx: %u\n - graphics_qfam_idx: %u\n - has_exts: %u\n - swapchain_met: %u", pdevice_num++, result.graphics_qfam_idx, result.present_qfam_idx, result.has_wanted_extensions, result.swapchain_props.meets_min_reqs);

	if(result.graphics_qfam_idx != NULL_QUEUE_FAM_INDEX && 
		 result.present_qfam_idx != NULL_QUEUE_FAM_INDEX  &&
		 result.has_wanted_extensions &&
		 result.swapchain_props.meets_min_reqs) {
		result.meets_min_reqs = true;
	}
	 
	// No ranking system yet: TODO
	result.score = 100;

	return result;
}

static b32 checkDeviceExtensions(VkPhysicalDevice device, const char **exts, u32 exts_cnt, Arena scratch) {

	u32 all_exts_cnt;
	VkExtensionProperties *all_exts;

	vkEnumerateDeviceExtensionProperties(device, NULL, &all_exts_cnt, NULL);
	all_exts = New(&scratch, VkExtensionProperties, all_exts_cnt);
	vkEnumerateDeviceExtensionProperties(device, NULL, &all_exts_cnt, all_exts);

	b32 found_all = true;
	for(size_t i = 0; i < exts_cnt; i++) {
		b32 found = false;
		for(size_t j = 0; j < all_exts_cnt; j++) {
			Str compare = StrFromCStr(all_exts[j].extensionName);
			Str wanted = StrFromCStr(exts[i]);
			if(StrEq(compare, wanted))
				found = true;
		}

		found_all = found_all && found;

		// TODO: Return array of missing extensions
		if(!found)
			log_warnf("WARN: Found a vulkan device that doesn't support extension '%s'", exts[i]);
	}

	return found_all;
}

static SwapchainProps querySwapchain(VkPhysicalDevice device, VkSurfaceKHR surface, b32 vsync_lock, Arena scratch) {
	VkSurfaceCapabilitiesKHR caps;
	VkSurfaceFormatKHR *fmts;
	VkPresentModeKHR *present_modes;
	u32 fmt_cnt, mode_cnt;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &caps);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fmt_cnt, NULL);
	fmts = New(&scratch, VkSurfaceFormatKHR, fmt_cnt);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fmt_cnt, fmts);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_cnt, NULL);
	present_modes = New(&scratch, VkPresentModeKHR, mode_cnt);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_cnt, present_modes);

	VkSurfaceFormatKHR sc_fmt;

	for(isize i = 0; i < fmt_cnt; i++)
		if(fmts[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
			 fmts[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				sc_fmt = fmts[i];

	// Default use vsync as it's always guaranteed to be available
	VkPresentModeKHR sc_mode = VK_PRESENT_MODE_FIFO_KHR;

	if(!vsync_lock)
		for(isize i = 0; i < mode_cnt; i++)
			if(present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				sc_mode = VK_PRESENT_MODE_MAILBOX_KHR;

	VkExtent2D sc_extent = caps.maxImageExtent;

	// TODO: Fetch window size when currentExtent is defined, and clamp between
	// min and max extents.
	//
	// Would be easier to do when a `Window` struct is defined - can attach surface
	// width and height to one function parameter
	if(caps.currentExtent.width != UINT32_MAX &&
		 caps.currentExtent.height != UINT32_MAX) {
		sc_extent = caps.currentExtent;
	}

	// Prefer triple-buffering, otherwise take the min
	u32 sc_img_cnt = (3 > caps.minImageCount) ? caps.minImageCount : 3;

	// Just double-check we aren't over the maximum
	// Don't think this is necessary
	if(caps.maxImageCount != 0 /* = 0 means we have inf images */ &&
		 sc_img_cnt > caps.maxImageCount) {
		sc_img_cnt = caps.maxImageCount;
	}

	return (SwapchainProps) {
			.fmt = sc_fmt,
			.mode = sc_mode,
			.extent = sc_extent,
			.img_cnt = sc_img_cnt,
			.pre_transform = caps.currentTransform,

			.meets_min_reqs = (fmt_cnt > 0 && mode_cnt > 0)
	};
}

static b32 checkLayers(const char** layers, u32 layer_cnt, Arena scratch) {
  u32 all_layer_cnt;
	VkLayerProperties *all_layers;

	// Look at all the possible layers we can load,
	// to see if we can load our desired layers
  vkEnumerateInstanceLayerProperties(&all_layer_cnt, NULL);
	all_layers = New(&scratch, VkLayerProperties, layer_cnt);
  vkEnumerateInstanceLayerProperties(&all_layer_cnt, all_layers);

	b32 found_all = true;
  // Search for the layers we want
  // O(n^2) Tasty!
  for (size_t i = 0; i < layer_cnt; i++) {
    b32 found = false;

    for (size_t j = 0; j < layer_cnt; j++) {
			Str compare = StrFromCStr(all_layers[j].layerName);
			Str wanted = StrFromCStr(layers[i]);

      if (StrEq(compare, wanted)) {
        found = true;
        break;
      }
    }

    if (!found) {
      log_warnf("Layer required, but doesn't exist: %s", layers[i]);
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
		vkDestroySwapchainKHR(device->vk_dev, device->vk_swapchain, NULL);
		vkDestroyDevice(device->vk_dev, NULL);
	}

  PFN_vkDestroyDebugUtilsMessengerEXT func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          _vk_instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != NULL) {
    func(_vk_instance, _vk_dbg_messenger, NULL);
  }
  vkDestroyInstance(_vk_instance, NULL);
}
