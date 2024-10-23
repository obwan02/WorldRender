#include "core.h"

void * alloc(struct arena *a, isize size, isize align, isize count, i32 flags) {
	isize avail = a->end - a->beg;
	isize pad = -(uptr)a->beg & (align - 1);

	if(flags & ALLOC_SOFT_FAIL) {
		return NULL;
	} else {
		// TODO: Have better OOM policy
		hard_assert(count <= (avail - pad)/size);
	}

	isize byte_count = size * count;
	u8 *result = a->beg + pad;

	a->beg += pad + byte_count;

	return result;
}

void __break(void) {

	// Inspired by the scottt/debugbreak repo:
	// https://github.com/scottt/debugbreak

#ifdef _MSC_VER
	__debugbreak();
#else

#if __i386__ || __x86_64
	__asm__ volatile("int $0x03");
#elif __thumb__
	__asm__ volatile(".inst 0xde01");
#elif __arm__ && !__thumb__
	__asm__ volatile(".inst 0xe7f001f0");
#elif __aarch64__
	__asm__ volatile(".inst 0xd4200000");
#elif __powerpc__
	__asm__ volatile(".4byte 0x7d821008");
#elif __riscv
	__asm__ volatile(".4byte 0x00100073");
#else
#error Cannot generate debug break for target architecture
#endif
	
#endif // ifdef _MSC_VER

}
