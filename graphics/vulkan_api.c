/* Creation Date: 1/10/2023
 * Creator: obwan02
 * ========================
 *
 * Vulkan graphics header
 *
 */
#include "vulkan_api.h"
#include "platform/platform.h"
#include <vulkan/vulkan_core.h> 
#include <vulkan/vk_enum_string_helper.h>

#define NULL_QUEUE_FAM_INDEX UINT32_MAX

// TODO: Make work for multi-threaded code
static VkInstance _vk_instance = VK_NULL_HANDLE;
static VkDebugUtilsMessengerEXT _vk_dbg_messenger;

static const char *wanted_instance_exts[] = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};

static const char *wanted_device_exts[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static const char *wanted_layers[] = {
	"VK_LAYER_KHRONOS_validation",
};

struct swapchain_props {
	b32 meets_min_reqs;
	
	VkSurfaceFormatKHR fmt;
	VkPresentModeKHR mode;
	VkExtent2D extent;
	VkSurfaceTransformFlagBitsKHR pre_transform;
	u32 img_cnt;
};

struct phydev_props {
	i32 score;
	
	u32 graphics_qfam_idx;
	u32 present_qfam_idx;

	struct swapchain_props swapchain_props;

	b32 meets_min_reqs;
	b32 has_wanted_extensions;
};

static b32 check_device_extensions(VkPhysicalDevice, const char**, u32, struct arena);
static b32 check_layers(const char **, u32, struct arena);
static void query_swapchain(VkPhysicalDevice, VkSurfaceKHR, b32, struct swapchain_props *, struct arena);
static void query_phydev_props(VkPhysicalDevice, VkSurfaceKHR, b32, struct phydev_props *, struct arena);


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
		log_dbgf(":: [VK VALIDATION LAYER] %s", pcallback_data->pMessage);

	else if(msg_sev & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		log_warnf(":: [VK VALIDATION LAYER] %s", pcallback_data->pMessage);
	
	else 
		log_errf(":: [VK VALIDATION LAYER] %s", pcallback_data->pMessage);
	

  return 0;
}

VkInstance _gvk_get_instance() {
	return _vk_instance;
}

i32 _gvk_init(struct str app_name, i32 ver_maj, i32 ver_minor, i32 ver_patch, u32 platform_exts_count, const char **platform_exts, b32 portable_flag, struct arena scratch) {
							  
	hard_assert(app_name.str != NULL);
	if (platform_exts_count) {
		hard_assert(platform_exts != NULL);
	}

	VkDebugUtilsMessengerCreateInfoEXT dbg_callback_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
		        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
		        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
		        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = _vkDebugCallback,
		.pUserData = NULL
	};


	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = (const char *) app_name.str,
		.applicationVersion = VK_MAKE_API_VERSION(0, ver_maj, ver_minor, ver_patch),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_API_VERSION(0, ver_maj, ver_minor, ver_patch),
		.apiVersion = VK_API_VERSION_1_0,
	};


	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.pNext = &dbg_callback_create_info // Have debugging before instance is created
	};

	b32 should_add_portable_ext = FALSE;

	if(portable_flag) {
		b32 has_portable_ext = FALSE;
		for(isize i = 0; i < platform_exts_count; i++) {
			if(wrld_cstreq(platform_exts[i], VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
				has_portable_ext = TRUE;
				break;
			}
		}

		for(isize i = 0; i < COUNT_OF(wanted_instance_exts); i++) {
			if(wrld_cstreq(wanted_instance_exts[i], VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
				has_portable_ext = TRUE;
				break;
			}
		}

		should_add_portable_ext = !has_portable_ext;
	}

	isize all_extensions_count = COUNT_OF(wanted_instance_exts) + platform_exts_count + (should_add_portable_ext ? 1 : 0);
	const char **all_extensions = arena_alloc(&scratch, const char *, all_extensions_count, ALLOC_NORMAL);

	wrld_memcpy(all_extensions, wanted_instance_exts, sizeof(wanted_instance_exts));
	wrld_memcpy(all_extensions + COUNT_OF(wanted_instance_exts), platform_exts, platform_exts_count * sizeof(const char *));
	if(portable_flag && should_add_portable_ext) {
		log_dbg("Adding portability extension, as the portable flag is set.");
		all_extensions[all_extensions_count - 1] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
	}

	log_dbg("Vulkan Instance Extensions Requested: ");
	for(isize i = 0; i < all_extensions_count; i++) {
		log_dbgf("\t- %s", all_extensions[i]);
	}

	instance_create_info.enabledExtensionCount = all_extensions_count;
	instance_create_info.ppEnabledExtensionNames = all_extensions;

	log_dbg("Vulkan Instance Layers Requested: ");
	for(isize i = 0; i < COUNT_OF(wanted_layers); i++) {
		log_dbgf("\t- %s", wanted_layers[i]);
	}
	// Ignore any errors relating to not loading
	// layers :)
	if(!check_layers(wanted_layers, COUNT_OF(wanted_layers), scratch)) {
		log_err("Couldn't load all requested layers, cannot initialise vulkan!");
		return ERR_ENVIRON;
	}
	instance_create_info.ppEnabledLayerNames = wanted_layers;
	instance_create_info.enabledLayerCount = COUNT_OF(wanted_layers);

	if(portable_flag) {
		instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
	
	// Create the instance
	VkResult result = vkCreateInstance(&instance_create_info, NULL, &_vk_instance);
	if (result != VK_SUCCESS) {
		log_err("Failed to create vkInstance");
		return ERR_UNKNOWN;
	}

	// Setup debugging callbacks
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vk_instance, "vkCreatDebugUtilsMessengerEXT");
	if (vkCreateDebugUtilsMessengerEXT && vkCreateDebugUtilsMessengerEXT(_vk_instance, &dbg_callback_create_info, NULL, &_vk_dbg_messenger) != VK_SUCCESS) {
		log_err("Failed to register vulkan debug callback");
		return ERR_UNKNOWN;
	}

	log_dbg("Vulkan Initialised!");
	return NOERR;
}

i32 _gvk_device_init(VkSurfaceKHR surface, b32 vsync_lock, b32 portable_flag, struct gdevice *out_device, struct arena *perm, struct arena scratch) {
	
	// Create the surface that the device will draw to
	u32 device_count = 0;
	VkPhysicalDevice* devices = NULL;
	VkResult vkresult = VK_SUCCESS;

	vkEnumeratePhysicalDevices(_vk_instance, &device_count, NULL);
	hard_assert(device_count >= 1);
	devices = arena_alloc(&scratch, VkPhysicalDevice, device_count, ALLOC_NORMAL);
	vkEnumeratePhysicalDevices(_vk_instance, &device_count, devices);

	// Pick the best device that is suitable for the job
	// TODO: Allow user to override decision made by us
	
	i32 max_score = -100;
	VkPhysicalDevice chosen_dev = VK_NULL_HANDLE;
	struct phydev_props chosen_dev_props;

	log_dbgf("Found %u physical devices", device_count);
	for(int i = 0; i < device_count; i++) {

		struct phydev_props props;
		query_phydev_props(devices[i], surface, vsync_lock, &props, scratch);
		if(props.meets_min_reqs && props.score > max_score) {
			chosen_dev = devices[i];
			chosen_dev_props = props;
		}
	}

	if(chosen_dev == VK_NULL_HANDLE) {
		// TODO: Maybe remove logging and replace with a return param indicating
		// how many physical devices couldn't be found
		log_err("Couldn't find any physical devices that meet minimum requirements");
		return ERR_ENVIRON;
	}

	log_dbgf("Graphics Queue Family Index: %u", chosen_dev_props.graphics_qfam_idx);
	log_dbgf("Present Queue Family Index: %u", chosen_dev_props.present_qfam_idx);

	float q_priority = 1.0f;
	u32 all_qfam_idxs[] = {chosen_dev_props.graphics_qfam_idx, chosen_dev_props.present_qfam_idx};

	// Not using whole array, have to keep track of # items
	VkDeviceQueueCreateInfo q_create_infos[COUNT_OF(all_qfam_idxs)];
	isize q_create_infos_cnt = 0;

	for(isize i = 0; i < COUNT_OF(all_qfam_idxs); i++) {

		b32 dup = FALSE;
		for(isize j = 0; j < q_create_infos_cnt; j++) {
			if(all_qfam_idxs[i] == q_create_infos[j].queueFamilyIndex) dup = TRUE;
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
	VkDeviceCreateInfo create_info = {
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
	hard_assert(check_device_extensions(chosen_dev, wanted_device_exts, COUNT_OF(wanted_device_exts), scratch));
	hard_assert(chosen_dev_props.swapchain_props.meets_min_reqs);

	isize all_exts_count = COUNT_OF(wanted_device_exts);
	const char **all_exts = wanted_device_exts;
	if(portable_flag) {
		all_exts_count += 1;
		all_exts = arena_alloc(&scratch, const char*, all_exts_count, ALLOC_NORMAL);
		wrld_memcpy(all_exts, wanted_device_exts, sizeof(wanted_device_exts));
		all_exts[all_exts_count - 1] = "VK_KHR_portability_subset";
	}

	create_info.ppEnabledExtensionNames = all_exts;
	create_info.enabledExtensionCount = all_exts_count;

	VkDevice logical_dev;
	vkresult = vkCreateDevice(chosen_dev, &create_info, NULL, &logical_dev);
	if(vkresult != VK_SUCCESS) {
		log_errf("Failed to create logical vulkan device: %s", string_VkResult(vkresult));
		return ERR_UNKNOWN;
	}

	VkQueue graphics_q, present_q;
	vkGetDeviceQueue(logical_dev, chosen_dev_props.graphics_qfam_idx, 0, &graphics_q);
	vkGetDeviceQueue(logical_dev, chosen_dev_props.present_qfam_idx, 0, &present_q);

	// PICKUP: Creating VkSwapChainKHR :))
	struct swapchain_props *sc_props = &chosen_dev_props.swapchain_props;

	b32 qfam_same = chosen_dev_props.present_qfam_idx == chosen_dev_props.graphics_qfam_idx;
	u32 qfam_idxs[] = {chosen_dev_props.present_qfam_idx, chosen_dev_props.graphics_qfam_idx};

	VkSwapchainCreateInfoKHR sc_create_inf = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
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
			.oldSwapchain = VK_NULL_HANDLE,
			.surface = surface,
	};


	VkSwapchainKHR swapchain;
	vkresult = vkCreateSwapchainKHR(logical_dev, &sc_create_inf, NULL, &swapchain);
	if(vkresult != VK_SUCCESS) {
		log_errf("Failed to create vulkan swapchain: %s", string_VkResult(vkresult));
		return ERR_UNKNOWN;
	}

	u32 sc_img_count = 0;
	vkGetSwapchainImagesKHR(logical_dev, swapchain, &sc_img_count, NULL);
	VkImage *sc_imgs = arena_alloc(perm, VkImage, sc_img_count, ALLOC_NORMAL);
	VkImageView *sc_views = arena_alloc(perm, VkImageView, sc_img_count, ALLOC_NORMAL);
	vkGetSwapchainImagesKHR(logical_dev, swapchain, &sc_img_count, sc_imgs);

	for(isize i = 0; i < sc_img_count; i++) {
		VkImageViewCreateInfo img_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = sc_imgs[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = sc_props->fmt.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},

			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1, 
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vkCreateImageView(logical_dev, &img_view_create_info, NULL, sc_views + i);
	}

	*out_device = (struct gdevice) {
		.vsync_lock = vsync_lock,
		.vk_dev = logical_dev,
		.vk_graphics_q = graphics_q,
		.vk_present_q = present_q,
		.vk_surface = surface,
		.vk_swapchain = swapchain,
		.vk_swapchain_imgs = sc_imgs,
		.vk_swapchain_img_count = sc_img_count,
		.vk_swapchain_fmt = sc_props->fmt.format,
		.vk_swapchain_extent = sc_props->extent,
		.vk_swapchain_img_views = sc_views,
	};

	return NOERR;

}

static void query_phydev_props(VkPhysicalDevice device, VkSurfaceKHR surface, b32 vsync_lock, struct phydev_props *out_props, struct arena scratch) {
	VkPhysicalDeviceProperties dev_props;
	VkPhysicalDeviceFeatures dev_feats;

	vkGetPhysicalDeviceProperties(device, &dev_props);
	vkGetPhysicalDeviceFeatures(device, &dev_feats);

	struct phydev_props result = {
		.has_wanted_extensions = FALSE,
		.meets_min_reqs = FALSE,
		.graphics_qfam_idx = NULL_QUEUE_FAM_INDEX,
		.present_qfam_idx = NULL_QUEUE_FAM_INDEX,
		.score = -1
	};

	u32 qfam_count = 0;
	VkQueueFamilyProperties *qfams = NULL;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &qfam_count, NULL);
	hard_assert(qfam_count >= 1);
	qfams = arena_alloc(&scratch, VkQueueFamilyProperties, qfam_count, ALLOC_NORMAL);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &qfam_count, qfams);

	for(size_t i = 0; i < qfam_count; i++) {
		if(qfams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			result.graphics_qfam_idx = i;
		}

		VkBool32 queueFamilySupportsPresent = FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &queueFamilySupportsPresent);

		if(queueFamilySupportsPresent) {
			if(result.present_qfam_idx == NULL_QUEUE_FAM_INDEX) {
				result.present_qfam_idx = i;
			}

			// Prioritise keeping the present and graphics queue the same
			// TODO: Check if this is good or bad
			else if(result.present_qfam_idx == result.graphics_qfam_idx) {
				continue;
			}

			result.present_qfam_idx = i;
		}
	}

	result.has_wanted_extensions = check_device_extensions(device, wanted_device_exts, COUNT_OF(wanted_device_exts), scratch);
	query_swapchain(device, surface, vsync_lock, &result.swapchain_props, scratch);

	// TODO: Re-enable for non macOS. M2 doesn't seem to support geometryShaders???
	// result.meetsMinsRequirements = deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	static i32 pdevice_num = 0;
	log_dbgf("Physical Device %d\n - present_qfam_idx: %u\n - graphics_qfam_idx: %u\n - has_exts: %u\n - swapchain_met: %u", pdevice_num++, result.graphics_qfam_idx, result.present_qfam_idx, result.has_wanted_extensions, result.swapchain_props.meets_min_reqs);

	if(result.graphics_qfam_idx != NULL_QUEUE_FAM_INDEX && 
		 result.present_qfam_idx != NULL_QUEUE_FAM_INDEX  &&
		 result.has_wanted_extensions &&
		 result.swapchain_props.meets_min_reqs) {
		 result.meets_min_reqs = TRUE;
	}
	 
	// No ranking system yet: TODO
	result.score = 100;
	*out_props = result;
}

static b32 check_device_extensions(VkPhysicalDevice device, const char **exts, u32 exts_count, struct arena scratch) {

	u32 all_exts_count;
	VkExtensionProperties *all_exts;

	vkEnumerateDeviceExtensionProperties(device, NULL, &all_exts_count, NULL);
	all_exts = arena_alloc(&scratch, VkExtensionProperties, all_exts_count, ALLOC_NORMAL);
	vkEnumerateDeviceExtensionProperties(device, NULL, &all_exts_count, all_exts);

	b32 found_all = TRUE;
	for(size_t i = 0; i < exts_count; i++) {
		b32 found = FALSE;
		for(size_t j = 0; j < all_exts_count; j++) {
			const char *compare = all_exts[j].extensionName;
			const char *wanted = exts[i];

			if(wrld_cstreq(compare, wanted)) {
				found = TRUE;
			}
		}

		found_all = found_all && found;

		// TODO: Return array of missing extensions
		if(!found)
			log_warnf("WARN: Found a vulkan device that doesn't support extension '%s'", exts[i]);
	}

	return found_all;
}

void query_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface, b32 vsync_lock, struct swapchain_props *out_props, struct arena scratch) {
	VkSurfaceCapabilitiesKHR caps;
	VkSurfaceFormatKHR *fmts;
	VkPresentModeKHR *present_modes;
	u32 fmt_count, mode_count;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &caps);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fmt_count, NULL);
	fmts = arena_alloc(&scratch, VkSurfaceFormatKHR, fmt_count, ALLOC_NORMAL);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fmt_count, fmts);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, NULL);
	present_modes = arena_alloc(&scratch, VkPresentModeKHR, mode_count, ALLOC_NORMAL);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, present_modes);

	hard_assert(fmt_count > 0);
	VkSurfaceFormatKHR sc_fmt = fmts[0];

	for(isize i = 0; i < fmt_count; i++)
		if((fmts[i].format == VK_FORMAT_B8G8R8A8_SRGB ||
				fmts[i].format == VK_FORMAT_R8G8B8A8_SRGB) &&
			 fmts[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				sc_fmt = fmts[i];

	// Default use vsync as it's always guaranteed to be available
	VkPresentModeKHR sc_mode = VK_PRESENT_MODE_FIFO_KHR;

	if(!vsync_lock)
		for(isize i = 0; i < mode_count; i++)
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
	u32 sc_img_count = 3 < caps.minImageCount ? caps.minImageCount : 3;
	log_dbgf("Using %d images for the swapchain", sc_img_count);

	// Just double-check we aren't over the maximum
	// Don't think this is necessary
	if(caps.maxImageCount != 0 /* = 0 means we have inf images */ &&
			sc_img_count > caps.maxImageCount) {
		sc_img_count = caps.maxImageCount;
	}

	*out_props = (struct swapchain_props) {
		.fmt = sc_fmt,
		.mode = sc_mode,
		.extent = sc_extent,
		.img_cnt = sc_img_count,
		.pre_transform = caps.currentTransform,

		.meets_min_reqs = (fmt_count > 0 && mode_count > 0)
	};
}

static b32 check_layers(const char** layers, u32 layer_count, struct arena scratch) {
	u32 all_layer_count;
	VkLayerProperties *all_layers;

	// Look at all the possible layers we can load,
	// to see if we can load our desired layers
	vkEnumerateInstanceLayerProperties(&all_layer_count, NULL);
	all_layers = arena_alloc(&scratch, VkLayerProperties, all_layer_count, ALLOC_NORMAL);
	vkEnumerateInstanceLayerProperties(&all_layer_count, all_layers);


	b32 found_all = TRUE;
	// Search for the layers we want
	// O(n^2) Tasty!
	for (isize i = 0; i < layer_count; i++) {
		b32 found = FALSE;

		for (isize j = 0; j < all_layer_count; j++) {
			const char *compare = all_layers[j].layerName;
			const char *wanted = layers[i];

			if(wrld_cstreq(compare, wanted)) {
				found = TRUE;
				break;
			}
		}

		if (!found) {
			log_warnf("Layer required, but doesn't exist: %s", layers[i]);
			found_all = FALSE;
		}
	}

	return found_all;
}

void CreateShader(void) {


}

// TODO: Get rid of device from here
// Maybe have some kind of automatic cleanup for vulkan objects??
// Or maybe just store the device globally if we assume we only have one
// device
void _gvk_device_cleanup(struct gdevice *device) {
	if(device != NULL) {
		for(isize i = 0; i < device->vk_swapchain_img_count; i++) {
			vkDestroyImageView(device->vk_dev, device->vk_swapchain_img_views[i], NULL);
		}

		vkDestroySwapchainKHR(device->vk_dev, device->vk_swapchain, NULL);
		vkDestroySurfaceKHR(_vk_instance, device->vk_surface, NULL);
		vkDestroyDevice(device->vk_dev, NULL);
	}

	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vk_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(_vk_instance, _vk_dbg_messenger, NULL);
	}

	vkDestroyInstance(_vk_instance, NULL);
}
