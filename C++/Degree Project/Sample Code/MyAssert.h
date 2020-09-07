
#pragma once

#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#undef MY_ASSERT

#if !defined(NDEBUG)
#if defined(_WIN32) // Windows 32/64-bit
#include <windows.h>
#define DBGBREAK() DebugBreak();
#else
#define DBGBREAK() __builtin_trap();
#endif
#define MY_ASSERT(expr) if (!(expr)) DBGBREAK()
#else
#define MY_ASSERT(expr)
#endif

#endif // MY_ASSERT_H
