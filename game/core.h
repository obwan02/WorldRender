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

// Represents a String
//
// Internally, an invariant is held, (and must be upheld)
// such that a null char must exist at str.str[str.len]
struct str {
	u8 *str;
	usize len;
};

// Arena design inpsired (copied from :))  
// https://nullprogram.com/blog/2023/09/27/.

#define ALLOC_HARD_FAIL   1
#define ALLOC_NOZERO      2

#define arena_safe_new(arena, T, n, flags) (T *) Alloc((arena), sizeof(T), _Alignof(T), (n), flags)
#define arena_new(arena, T, n) (T *) Alloc((arena), sizeof(T), _Alignof(T), (n), 0)

// arena
struct arena {
	u8 *beg, *end;
};

void * arena_alloc(struct arena *a, isize size, isize align, isize count, i32 flags);

#define hard_assert(expr)                                                                                                                          \
  if (!(expr)) {                                                                                                                                   \
    // TODO: replace with custom io impl													   \
    puts("Assert failed: '" #expr "' line " STRINGIFY_EXPR(__LINE__) ", file " STRINGIFY_EXPR(__FILE__));					   \
    __break();                                                                                                                                     \
  }

void __break(void);
