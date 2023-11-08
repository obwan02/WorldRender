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
#include <uchar.h>
#include <stddef.h>

#define STRINGIFY(x) #x
#define STRINGIFY_EXPR(x) STRINGIFY(x)

#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))

#define log_err(x) fprintf(stderr, x "\r\n")
#define log_errf(x, ...) fprintf(stderr, x "\r\n", __VA_ARGS__)
#define log_warnf(x, ...) fprintf(stderr, x "\r\n", __VA_ARGS__)

#define true  ((b32)1)
#define false ((b32)0)

typedef uint8_t   u8;
typedef char16_t  c16;
typedef int32_t   b32;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
typedef uintptr_t uptr;
typedef char      byte;
typedef ptrdiff_t isize;
typedef size_t    usize;

#define S(x) (Str) { .str = (u8 *)(x), .len = sizeof(x) - 1 }
typedef struct Str {
	const u8 *str;
	usize len;
} Str;

typedef struct MutStr {
	u8 *str;
	usize len, cap;
} MutStr;

/// \see _StrEq
#define StrEq(a, b) _StrEq((Str){ (a).str, (a).len }, (Str){ (b).str, (b).len })
/// \see _StrJoin
#define StrJoin(a, b, out) _StrJoin((Str){ (a).str, (a).len }, (Str){ (b).str, (b).len }, (out))
/// \see _StrCpy
#define StrCpy(a, out) _StrCpy((Str){ (a).str, (a).len }, (out))

/**
 * Creates a \ref Str from a C style string.
 *
 * \param[in]		the C string
 * \return			the resulting \ref Str
 */
Str StrFromCStr(const char* str);

/**
 * Creates a \ref MutStr from a C style string.
 *
 * \param[in]		the C string
 * \param[in]		the capacity of the underlying memory the C string is occupying
 * \return			the resulting \ref MutStr
 */
MutStr MutStrFromCStr(char *str, size_t cap);

/** 
 * Compares 2 strings for equality.
 *
 * \param[in] a		the first string to compare
 * \param[in] b		the second string to compare
 * \return				`true` if a is equal to b, `false` otherwise
 */
b32 _StrEq(Str a, Str b);

/**
 * Concatenates 2 strings together, and stores the result
 * in the provided \ref MutString.
 *
 * \param[in] a			the start of the resultant string
 * \param[in] b			the end of the resultant string
 * \param[out] out	the \ref MutString to store the result into	
 */
b32 _StrJoin(Str a, Str b, MutStr out);

/**
 * Copies a string into another.
 *
 * \param[in] a			the string to be copied
 * \param[out] out	the string to be copied into
 * \return					`true` if the string is successfully copied. Will return `false`
 *									if there is not enough capacity in `out` to copy the input string over.
 */
b32 _StrCpy(Str a, MutStr out);

/**
 * \brief Ensures that a string is compatible with c-strings. 
 *
 * This is needed as \ref Str and \ref MutStr aren't guaranteed 
 * to contain a null char at the end of their string. The string will
 * remain a valid c-string until it is modified. Any modification operation
 * will remove the guarantee that the string is a valid c-string.\
 *
 * \note This operation does not modify the length of the string. However, if does
 * require that len < cap.
 
 *
 * \param[in, out] out	the string to be modified to ensure it has a null char at the end
 * \return							`true` if there is enough capacity in the string to add a null char, and 
 *											a null char was added. `false` if the null char wasn't added because there wasn't
 *											enough capacity.
 *
 */
b32 EnsureCStr(MutStr out);

void Copy(void *restrict dst, const void *restrict src, usize len);
