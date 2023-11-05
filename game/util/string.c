#include "string.h"
#include <string.h>

bool _StrEq(Str a, Str b) {
	if(a.len != b.len) return false;

	for(size_t i = 0; i < a.len; i++) {
		if(a.str[i] != b.str[i]) return false;
	}

	return true;
}

bool _StrCat(Str a, Str b, MutStr out) {
	if(a.len + b.len <= out.cap) {
		memcpy(out.str, a.str, a.len);
		memcpy(out.str + a.len, b.str, b.len);
		return true;
	}

	return false;
}

bool _StrCpy(Str a, MutStr out) {
	if(a.len <= out.cap) {
		memcpy(out.str, a.str, a.len);
		return true;
	}

	return false;
}

bool EnsureCStr(MutStr out) {
	if(out.len < out.cap) {
		out.str[out.len++] = '\0';
		return true;
	}

	return false;
}

void Test() {
	Str a = STR("Hello ");
	Str b = STR("World ");
	
	char abuffer[256];
	MutStr amut = { abuffer, 0, 256 };

	char buffer[256];
	MutStr out = { buffer, 0, 256 };

	StrCat(amut, b, out);
}
