# Graphics Api Notes
Notes on desired design for the graphics API. This will
evolve over time as I learn more about Vulkan, graphics APIs
and renderers.

## Goals
- Somehow have a method to specify desired extensions during
  intialisation, and configure behaviour based on loaded
  extensions.

- More graphics into it's own "Graphics Core" that can
  compiles separately.

## Ideas/Solutions??

- `InitGDevice` should take hints as to the number of queues of each type
- Somehow during `_GVkInit` we take a list of optional and
  required extensions, where optional extensions can be
  queried after loading.
