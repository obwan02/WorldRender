#pragma once

#include <stddef.h>
#include <stdbool.h>

#define STR(x) (Str) { .str = x, .len = sizeof(x) }

typedef struct Str {
	const char *str;
	size_t len;
} Str;

typedef struct MutStr {
	char *str;
	size_t len, cap;
} MutStr;

/// \see _StrEq
#define StrEq(a, b) _StrCat((Str){ (a).str, (a).len }, (Str){ (b).str, (b).len });
/// \see _StrCpy
#define StrJoin(a, b, out) _StrCat((Str){ (a).str, (a).len }, (Str){ (b).str, (b).len }, (out))
#define StrCpy(a, out) _StrCat((Str){ (a).str, (a).len }, (out))

/** 
 * Compares 2 strings for equality.
 *
 * \param[in] a		the first string to compare
 * \param[in] b		the second string to compare
 * \return				`true` if a is equal to b, `false` otherwise
 */
bool _StrEq(Str a, Str b);

/**
 * Concatenates 2 strings together, and stores the result
 * in the provided \ref MutString.
 *
 * \param[in] a			the start of the resultant string
 * \param[in] b			the end of the resultant string
 * \param[out] out	the \ref MutString to store the result into	
 */
bool _StrJoin(Str a, Str b, MutStr out);
bool _StrCpy(Str a, MutStr out);
bool EnsureCStr(MutStr out);

