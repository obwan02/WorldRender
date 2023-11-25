# Memory Notes

# Arenas

The current approach to arenas means that the separation
between *permanently* and *termporarily* allocated memory is
by changing the parameter of a function to accept a pointer
rather than copy the arena.

**PROS**
- Arena allocation is very easy/intuitive, and makes passing
  pointers out of functions easy
- Fairlyish simple to tell when a function is gonna return
  allocated memory?
- No need to free stuff manually


**CONS**
- Impossible for a function to force memory to be allocated
  permanently, because it is up to the caller. Can be bad in
  situations when the callee knows lifetime.

  - Option to get around this is to pass a permanent arena
    as well as a normal arena around functions, but
    honestly, kinda a pain in the ass, like theres 2 arenas
    for must functions that exist??

  - Could change to `thread-local` type of arenas, and make
    each function opaque as to allocations (losing most
    benefits from above). Or maybe could just make a
    *permanent* arena with that behaviour

- Impossible for the caller of a function to know if the
  callee accidentally leaked some memory, if a mutable
  version of the arena was passed in.

- What to do when a function wants to allocate permanent
  memory and temporary memory?? 

  I mean you'd want to allocate the permanent memory first,
  but that's not very intuitive...

## Thoughts on perm/temp arenas
Might just have to go with with 2 arena params honestly, and
just make sure there are enough examples to make it clear...

I mean it's not too often that you get a function both
temporarily and permanently allocating...

Only problem is that it kinda makes it non-obvious you can
use a scratch arena to return pointers? I guess
documentation can cover that 
