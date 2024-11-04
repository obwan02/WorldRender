#include "core.h"
#include "../platform/platform.h"

void * arena_aligned_alloc(struct arena *a, isize size, isize align, isize count, u32 flags) {
	isize available = a->end - a->start;
	isize pad = -(uptr)a->start & (align - 1);

	if(count > (available - pad)/size) {
		if(flags & ALLOC_HARD_FAIL) { hard_assert(false); }
		else { return NULL; }
	}

	isize byte_count = size * count;
	u8 *result = a->start + pad;

	a->start += pad + byte_count;
	if(!(flags & ALLOC_NOZERO)) {
		wrld_memset(result, 0, byte_count);
	}

	return result;
}

void arena_init(void *start, isize byte_count, struct arena *out) {
	*out = (struct arena) {
		.start = start,
		.end = start + byte_count
	};
}
