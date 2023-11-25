# Structure separation in G

## VkDevice / GDevice
After we have created a device, we need that device to
create a swapchain.

Additionally, we also need the surface to check if we have
found a `device` that can present to a surface, and we need
it for swapchain creation.

