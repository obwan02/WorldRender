/* Creation Date: 16/7/2023
 * Creator: obwan02
 * ========================
 *
 * This file contains some of the core
 * functions/definitions that are used
 *
 */
#pragma once

#include <cwctype>
#include <stdint.h>
#include <memory>

#define STRINGIFY(x) #x
#define STRINGIFY_EXPR(x) STRINGIFY(x)

#define NO_ERROR (nullptr)

class ErrorBase {
public:
	virtual ~ErrorBase() {};
	virtual void print() = 0;
};

typedef std::unique_ptr<ErrorBase> Error;

template<typename T>
inline Error AllocError(T&& error) {
	return std::make_unique<T>(error);
}
