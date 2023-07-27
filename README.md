# World-Scale Render Project

The aim of this project is to make a renderer that can
render object the size of worlds, including macroscopic and
microscopic details.


# Some End Goals
1. Have similar look/feel as modern graphics applications
2. Should be backend agnostic. This means eventually having
   Metal and Vulkan backends, and maybe eventually D3D.
3. Should use no additional libraries that are not needed,
   i.e. GLFW, Boost, C++ STL. (Some exceptions are stuff
   like std::is_trivial, which require compiler builtins)


# Design Decisions
0. Use `struct`s for plain data, and `class`es for classes.
   This means all `struct`s should be trivial
1. Simple decisions to get fast baseline performance. I.e.
   don't make an abstract base class unless you need to
2. Keep abstractions fairly simple, unless they need to be
   complex.
3. KISS
4. The end goals are fairly lofty, which means sacrifices
   need to made in places - especially where certain
   features are not necessary. For example, hot-reloading
   graphics api backends is something that can be foregone
   as it is not necessary.

   This also includes features such as full-on c++ iterators
   for every custom datastructures - that just takes up
   unecessary time.

6. Minimise, and elminate wherever possible, pre-processor
   directives that change code paths. Sometimes this is
   unavoidable, such as in `game/graphics/api.h`, however,
   it should be avoided. If it is implemented,
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
