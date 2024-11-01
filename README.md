# World-Scale Render Project

The aim of this project is to make a renderer that can
render object the size of worlds, including macroscopic and
microscopic details.


# Some End Goals
1. Have similar look/feel as modern graphics applications
2. Might eventually be backend agnostic. Starting with
   Vulkan and moving on from there.
3. Will *eventually* move away from 3rd party libraries,
   once the needs arises. Currently, the effort vs. reward
   is unclear if using i.e. GLFW, v.s. X11, WinAPI etc.
   **NOTE**: Fast, performance critical code should be hand
   written.


# Design Decisions
1. Simple decisions to get fast baseline performance.
2. KISS
3. Refactor often and sooner (don't take this overboard
   tho).
4. The end goals are fairly lofty, which means sacrifices
   need to made in places - especially where certain
   features are not necessary. For example, hot-reloading
   graphics api backends is something that can be foregone
   as it is not necessary.

5. Minimise, and elminate wherever possible, pre-processor
   directives that change code paths. If it is implemented,
   there should be clear documentation in both the file
   header comment, and at the site of the directive.

   This rule is in place to cut down on the complexity of
   cross-platform code intermingling (for a terrible
   example, check out GLFW code). Using pre-processor
   directives can make the number of possible code
   permutations sky-rocket.

   Ideally, when this rule is followed, the number of code
   permutations that can occur should should be equal to the
   number of platforms/architectures supported

6. Platform specific functionality should be clearly
   separated from core logic. If possible, most information
   provided to the core logic should be passed from the OS,
   to the core logic. There should rarely be situations in
   which the core logic depends on an OS primitive (this is
   because not all concepts are shared between OSes).

   For example, asset loading logic needs to interact with
   the OS in order to save/load data from a file system.
   This should (probably) be provided through the platform 
   layer, through function pointers. I.e. we can have some
   functions:
      - `void (*save_asset)(str name, u8* data, int len)`
      - `void (*query_asset)(str name, int* len);`
      - `void (*load_assert)(str name, u8* data_out);`

   Which the core logic can then call. The alternative to
   this would be to expose these functions the platform.h
   header, which would also be a good solution.


# Some Guidelines

- To make things simple, all platform related code should be
  resolved at compile time. (apart from maybe SIMD stuff)

- Platform depedent code should be written in its own file


## Platforms
- Generic is any platform that supports libc
- macOS is the macOS operating system
- metal is any platform that supports the Metal API
- linux is any platform that supports X11
- vulkan is any platform that supports vulkan

