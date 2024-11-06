#include "core/core.h"

#define MEMCMP_FAST 0
#define MEMCMP_CONST_TIME 1

// TODO: Expose perm_allocator_get()
// TODO: Figure out how memory allocation will function (i.e. main will be passed down a slab of
// memory from the OS)

void wrld_memcpy(void* dest, const void* src, isize byte_count);
void wrld_memset(void* dest, u8 value, isize byte_count);
b32 wrld_memeq(const void *a, const void * b, isize count, u32 mode);
void __break();
