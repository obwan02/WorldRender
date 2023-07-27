/* Creation Date: 14/7/2023
 * Creator: obwan02
 * ========================
 *
 * This files contains the definitions
 * for strings
 */
#pragma once
#include <stddef.h>

#define WLRD_STR(s) (::wrld::String::from_cstr(s))

namespace wrld {

struct String {
  constexpr static String from_cstr(const char *);

  const char *data;
  size_t len;
};

struct MutString {
  constexpr static MutString from_cstr(char *);

  constexpr operator String() {
    return String{
        .data = this->data,
        .len = this->len,
    };
  }

  const char *data;
  size_t len;
};

constexpr inline MutString MutString::from_cstr(char *data) {

  size_t i = 0;
  for (; data[i] != '\0'; i++) {
  }
  size_t length = i;

  return MutString{
      .data = data,
      .len = length,
  };
}

constexpr inline String String::from_cstr(const char *data) {

  size_t i = 0;
  for (; data[i] != '\0'; i++) {
  }
  size_t length = i;

  return String{
      .data = data,
      .len = length,
  };
}
} // namespace wrld

inline const wrld::String operator""_S(const char *data, size_t len) {

  return wrld::String{
      .data = data,
      .len = len,
  };
}
