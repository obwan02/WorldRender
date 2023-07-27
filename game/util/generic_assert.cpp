/* Creation Date: 5/7/2023
 * Creator: obwan02
 * ========================
 *
 * This file contains the generic implementation
 * of an assertion.
 */
#include "assert.h"

void __break() { *((volatile int *)0) = 0; }
