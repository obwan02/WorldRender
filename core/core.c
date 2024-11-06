#include "core.h"
#include "../platform/platform.h"

void * arena_aligned_alloc(struct arena *a, isize size, isize align, isize count, u32 flags) {
	isize available = a->end - a->start;
	isize pad = -(uptr)a->start & (align - 1);

	if(count > (available - pad)/size) {
		if(flags & ALLOC_HARD_FAIL) { hard_assert(FALSE); }
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

isize wrld_cstrlen(const char* str) {
	isize size = 0;
	while(*str++) ++size;

	return size;
}

b32 wrld_cstreq(const char *a, const char *b) {
	isize sizea = wrld_cstrlen(a);
	isize sizeb = wrld_cstrlen(b);

	if(sizea != sizeb) return FALSE;
	return wrld_memeq(a, b, sizea, MEMCMP_FAST);
}
