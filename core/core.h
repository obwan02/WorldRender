/* Creation Date: 16/7/2023
 * Creator: obwan02
 * ========================
 *
 * This file contains some of the core
 * functions/definitions that are used
 *
 */
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#define STRINGIFY(x) #x
#define STRINGIFY_EXPR(x) STRINGIFY(x)

#define COUNT_OF(x) ((isize)(sizeof(x) / sizeof((x)[0])))

#define log_dbg(x) fprintf(stdout, "[DBG] " x "\r\n")
#define log_dbgf(x, ...) fprintf(stdout, "[DBG] " x "\r\n", __VA_ARGS__)
#define log_err(x) fprintf(stderr, "[ERR] " x "\r\n")
#define log_errf(x, ...) fprintf(stderr, "[ERR] " x "\r\n", __VA_ARGS__)
#define log_warnf(x, ...) fprintf(stderr, "[WRN] " x "\r\n", __VA_ARGS__)

#define true  ((b32)1)
#define false ((b32)0)

typedef int8_t    i8;
typedef uint8_t   u8;
// typedef char16_t  c16;
typedef int32_t   b32;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
typedef uintptr_t uptr;
typedef ptrdiff_t isize;
typedef size_t    usize;

// Represents a string
//
// Internally, an invariant is held, (and must be upheld)
// such that a null char must exist at str.str[str.len]. This is to ensure
// compatibility with other C apis (Vulkan).
struct str {
	u8 *str;
	isize len;
};

// Arena design inpsired (copied from :))  
// https://nullprogram.com/blog/2023/09/27/.

#define ALLOC_HARD_FAIL   1
#define ALLOC_NOZERO      2

#ifndef WRLD_TRACE_ALLOC
#define arena_alloc(arena, T, n, flags) (T *) arena_aligned_alloc((arena), sizeof(T), _Alignof(T), (n), flags)
#else
// TODO: Implement allocation tracing
#error "Allocation tracing has not been implemented!"
#endif

// arena
struct arena {
	u8 *start, *end;
	// TOOD: Add pointer to that adds tracing
	// information.
};

void * arena_aligned_alloc(struct arena *a, isize size, isize align, isize count, u32 flags);
struct arena arena_init(void *start, isize byte_count);

#define hard_assert(expr) if (!(expr)) {                                                                                                           \
    // TODO: replace puts with custom IO impl																																																			 \
    puts("Assert failed: '" #expr "' line " STRINGIFY_EXPR(__LINE__) ", file " STRINGIFY_EXPR(__FILE__));																				   \
    __break();                                                                                                                                     \
  }
