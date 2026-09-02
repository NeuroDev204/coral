/*
 * Linux compatibility layer for Windows types
 */
#ifndef _LINUX_COMPAT_H_
#define _LINUX_COMPAT_H_

// Use stdint.h for both C and C++
#ifdef __cplusplus
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#else
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>
#endif

// Define NULL if not already defined
#ifndef NULL
#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL ((void*)0)
#endif
#endif

// Windows types that need Linux equivalents
#if !defined(_WIN32) && !defined(__ANDROID__)

#ifndef WPARAM
typedef uintptr_t WPARAM;
#endif

#ifndef LPARAM
typedef intptr_t LPARAM;
#endif

#ifndef HWND
typedef void* HWND;
#endif

#ifndef HINSTANCE
typedef void* HINSTANCE;
#endif

#ifndef DWORD
typedef uint32_t DWORD;
#endif

#ifndef HANDLE
typedef void* HANDLE;
#endif

#ifndef HRESULT
typedef int32_t HRESULT;
#endif

#ifndef S_OK
#define S_OK 0
#endif

#ifndef E_FAIL
#define E_FAIL -1
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Windows-like calling conventions (no-op on Linux)
#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef WINAPI
#define WINAPI
#endif

// File seeking
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

// Windows string pointer types
#ifndef LPCWSTR
typedef const wchar_t* LPCWSTR;
#endif
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef LPCTSTR
#ifdef UNICODE
typedef LPCWSTR LPCTSTR;
#else
typedef LPCSTR LPCTSTR;
#endif
#endif

// Security attributes
#ifndef LPSECURITY_ATTRIBUTES
typedef void* LPSECURITY_ATTRIBUTES;
#endif

#ifndef PT_WF
#if defined(_WIN32)
#define PT_WF L"%s"
#else
#define PT_WF L"%ls"
#endif
#endif

// Windows data types
#ifndef BYTE
typedef uint8_t BYTE;
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef FILETIME
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
#endif

// Windows conversion functions (stub implementations)
#ifndef _WIN32
static inline int _wtoi(const wchar_t* str) {
    return wcstol(str, NULL, 10);
}
#endif

// Message box constants (no-op on Linux)
#ifndef MB_OK
#define MB_OK 0
#endif

#endif // !_WIN32 && !__ANDROID__

#endif // _LINUX_COMPAT_H_
