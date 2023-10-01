/* Creation Date: 22/6/2023
 * Creator: obwan02
 * ========================
 *
 * This file defines the main classes that can be shared
 * generically across platforms.
 *
 * All other classes, must be implemented in platform
 * specific headers. However, specfic classes must be
 * implemented, and these classes must define some common
 * functionality that is generic across all platforms.
 *
 * API inspired by: https://alextardif.com/RenderingAbstractionLayers.html
 */
#pragma once

// NOTE: This is one of the
// few locations that pre-processer directives
// are used to define cross-platform functionality.
//
// This section should be kept short so that clarity
// is retained 
#ifdef WRLD_METAL
#include "metal_api.h"
#elif WRLD_VULKAN
#include "vulkan_api.h"
#else
#error Render API is not specified, please include (or make) \
a platform specific header (i.e. "macOS_api.h"), and include it \
in this file, with a pre-processor directive.
#endif
