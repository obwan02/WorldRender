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

6. Game / Game Engine should be a "service to the OS". This
   means there should be some platform layer glue that
   passes all the necessary information (such as
   foward-direction, memory, etc.). See Casey Muratori
   Handmade Hero stuff. *Still not entirely sure on this*

# Some Guidelines

- Platform-dependent code is sometimes necessary. When using
  platform specific code, first, write a generic version (if
  possible) that can run on all platforms. Then, write the
  platform specific version

- To make things simple, all platform related code should be
  resolved at compile time.

- Platform depedent code should be written in its own file


## Platforms
- Generic is any platform that supports libc
- macOS is the macOS operating system
- metal is any platform that supports the Metal API
- linux is any platform that supports X11
- vulkan is any platform that supports vulkan

