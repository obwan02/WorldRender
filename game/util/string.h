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
/// \see _StrJoin
#define StrJoin(a, b, out) _StrCat((Str){ (a).str, (a).len }, (Str){ (b).str, (b).len }, (out))
/// \see _StrCpy
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

/**
 * Copies a string into another.
 *
 * \param[in] a			the string to be copied
 * \param[out] out	the string to be copied into
 * \return					`true` if the string is successfully copied. Will return `false`
 *									if there is not enough capacity in `out` to copy the input string over.
 */
bool _StrCpy(Str a, MutStr out);

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
bool EnsureCStr(MutStr out);

