/* Creation Date: 5/7/2023
 * Creator: obwan02
 * ========================
 *
 * This file contains the definitions for
 * assertions
 */
#pragma once
#include "../core.h"
#include <stdio.h>

#define ASSERT(expr)                                                                                                                               \
  if (!(expr)) {                                                                                                                                   \
    puts("Assert failed: '" #expr "' line " STRINGIFY_EXPR(__LINE__) ", file " STRINGIFY_EXPR(__FILE__)); \
    __break();                                                                                                                                     \
  }

void __break();
