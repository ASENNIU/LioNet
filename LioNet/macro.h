/**
 * @file macro.h
 * @brief 常用宏的封装
 */
#ifndef __LIONET_MACRO_H__
#define __LIONET_MACRO_H__

#include <assert.h>
#include <string.h>
#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define LIONET_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define LIONET_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIONET_LIKELY(x) (x)
#define LIONET_UNLIKELY(x) (x)
#endif

/// 断言宏封装
#define LIONET_ASSERT(x)                              \
  if (LIONET_UNLIKELY(!(x))) {                        \
    LIONET_ERROR(LIONET_LOG_ROOT())                   \
        << "ASSERTION: " #x << "\nbacktrace:\n"       \
        << LioNet::BacktraceToString(100, 2, "    "); \
    assert(x);                                        \
  }

/// 断言宏封装
#define LIONET_ASSERT2(x, w)                          \
  if (LIONET_UNLIKELY(!(x))) {                        \
    LIONET_ERROR(LIONET_LOG_ROOT())                   \
        << "ASSERTION: " #x << "\n"                   \
        << w << "\nbacktrace:\n"                      \
        << LioNet::BacktraceToString(100, 2, "    "); \
    assert(x);                                        \
  }

#endif
