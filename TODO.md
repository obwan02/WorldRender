# TODOs!

DOING:
- utilities


## Build up proper utilities

- To be able to use our programs wo/ libc, we need a proper
  set of utilities, such as memcpy, memset, etc.

Currently, this is solved by asking the platform layer to
provide them. This is fine ig, and allows for performant
solutions that take advantange of AVX.

Honestly it's a good enough method to move forwards with for
now.

Utilities needed:
- [ ] `memset`
- [ ] `memcpy`
- [ ] `memcmp`


## Separate builds, and compile wo/ libc

- All parts of the program, apart from files in platform.c
  shouldn't depend on libc.

- A solution would be to compile all parts of the program
  that don't depend on libc wo/ libc.


Currently, we just work off of vibes, and assume that we're
not pulling in libc dependencies where we're not supposed
to. Fixing this issue would make using libc a hard error.

### Remove all dependency on libc

We might be able to remove all dependency on libc if we
decide to make the platform entry points not depend on libc.

## Define a proper structure for memory management

- Currently just working with arenas

- Need to decide if OS passes down a fixed size memory block
  or we allocate on demand

- Need to decide what types of allocators we want to have
  available

- We, in the near future, would want an API that can just
  grab the global allocator from anywhere, and doesn't have
  to pass it down as a function parameter.
