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

#define STRINGIFY(x) #x
#define STRINGIFY_EXPR(x) STRINGIFY(x)

#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))

typedef int Result;

#define RESULT_SUCCESS 0
#define RESULT_GENERIC_ERR 1

#define log_err(x) fprintf(stderr, x "\r\n")
#define log_errf(x, ...) fprintf(stderr, x "\r\n", __VA_ARGS__)

// TODO: Possibly arrange for errors to be allocated on an arena rather than
// modified through parameters. In this manner, we can make an arena with a maximum
// allocation size, to avoid accidentally allocating too much memory.
