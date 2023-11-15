#include "core.h"

Str StrFromCStr(const char* str) {
	size_t len = 0;
	for(; str[len]; len++) {}

	return (Str) { .str = (const u8 *)str, .len = len };
}

MutStr MutStrFromCStr(char *str, size_t cap) {
	size_t len = 0;
	for(; str[len]; len++) {}

	return (MutStr) { .str = (u8 *)str, .len = len, .cap = cap };
}

b32 _StrEq(Str a, Str b) {
	if(a.len != b.len) return false;

	for(size_t i = 0; i < a.len; i++) {
		if(a.str[i] != b.str[i]) return false;
	}

	return true;
}

b32 _StrJoin(Str a, Str b, MutStr out) {
	if(a.len + b.len <= out.cap) {
		MemCopy(out.str, a.str, a.len);
		MemCopy(out.str + a.len, b.str, b.len);
		return true;
	}

	return false;
}

b32 _StrCpy(Str a, MutStr out) {
	if(a.len <= out.cap) {
		MemCopy(out.str, a.str, a.len);
		return true;
	}

	return false;
}

b32 EnsureCStr(MutStr out) {
	if(out.len < out.cap) {
		out.str[out.len] = '\0';
		return true;
	}

	return false;
}

#if __GNUC__ || __clang__
__attribute((malloc, alloc_size(2, 4), alloc_align(3)))
#endif
void *Alloc(Arena *a, isize size, isize align, isize count, i32 flags) {
	isize avail = a->end - a->beg;
	isize pad = -(uptr)a->beg & (align - 1);

	if(flags & ALLOC_SOFT_FAIL) {
		return NULL;
	} else {
		// TODO: Figure out "out-of-memory" policy
		ASSERT(count <= (avail - pad)/size);
	}

	isize byte_count = size * count;
	byte *result = a->beg + pad;

	a->beg += pad + byte_count;

	return (flags & ALLOC_NO_ZERO) ? result : MemSet(result, 0, byte_count);
}

void MemCopy(void *restrict dst1, const void *restrict src1, isize len) {
	u8 *restrict dst = dst1;
	const u8 *restrict src = src1;

	for(usize i = 0; i < len; i++)
		dst[i] = src[i];
}

void *MemSet(void *ptr, u8 val, isize count) {
	byte *byte_ptr = ptr;

	for(isize i = 0; i < count; i++)
		byte_ptr[i] = val;

	return ptr;
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
