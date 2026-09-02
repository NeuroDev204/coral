/*
Coral
Copyright (C) 2025  Coral

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _CODEDEFS_H_
#define _CODEDEFS_H_  

/*
 * This is to get rid of the following warnings:
 * warning C4996: 'sprintf' was declared deprecated
 *   see declaration of 'sprintf. This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_DEPRECATE. See online help for details.'
 */
#define _CRT_SECURE_NO_DEPRECATE  
#define _CRT_NON_CONFORMING_SWPRINTFS

/* Add memory leak detection to debug versions (Requires Pre-Processor Define of _CRTDBG_MAP_ALLOC*/
#include <stdlib.h>

#if defined(_WIN32)
#include <crtdbg.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#else
#include <stdint.h>
#include <wchar.h>
#include <stdio.h>
typedef unsigned char BYTE;
typedef uint32_t DWORD;
typedef void* HINSTANCE;
typedef void* HANDLE;
typedef void* HWND;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef const char* LPCTSTR;
typedef const wchar_t* LPCWSTR;
typedef void* LPSECURITY_ATTRIBUTES;
typedef int64_t __int64;
struct FILETIME { uint32_t dwLowDateTime; uint32_t dwHighDateTime; };
#define _wtoi(x) wcstol(x, NULL, 10)
#endif

/*
 * MSVC wide printf treats %s as wchar_t*. POSIX swprintf treats %s as char*
 * and %ls as wchar_t*. Using %s with a wchar_t* on Linux copies only the first
 * character (the following UTF-32 0 byte terminates the C string), which
 * collapses distinct registry keys such as byAll/byAmb onto the same slot.
 */
#ifndef PT_WF
#if defined(_WIN32)
#define PT_WF L"%s"
#else
#define PT_WF L"%ls"
#endif
#endif

/* 
 * The following define will temporarily turn off memory tracing.  This is useful when
 * we want to test the threaded tasks in debug mode. (With memory tracing on we have to disable
 * threading due to a limitation of the library)
 */
#define PT_TURN_OFF_MEM_TRACE

#ifndef PT_TURN_OFF_MEM_TRACE
  #ifdef __cplusplus
    #ifndef NO_PT_MEM_TRACE
    #include "ptMemTrace.h"
    #endif //NO_PT_MEM_TRACE
  #endif //__cplusplus
#endif //PT_TURN_OFF_MEM_TRACE

/* 
 * Next lines set whether functions are declared for static libs or dlls 
 * For function declarations in function source code, using DLL's 
 */
#ifdef PT_EXPORT_DLL 
	#define PT_DECLSPEC __declspec( dllexport )
#endif //PT_EXPORT_DLL 

/* For function declarations calling application source code, using DLL's */
#ifdef PT_IMPORT_DLL
	#define PT_DECLSPEC __declspec( dllimport )
#endif //PT_IMPORT_DLL

/* Static Library case */
#ifndef PT_DECLSPEC
	#define PT_DECLSPEC
#endif //PT_DECLSPEC

/* Widen string macros */
#define PT_WIDEN2(x) L ## x
#define PT_WIDEN(x) PT_WIDEN2(x)

/* Wide versions of __FILE__ macro */
#define PT_WFILE PT_WIDEN(__FILE__)

/* Wide string version of __LINE__ macro */
#define PT_STRINGIZE(x) PT_STRINGIZE2(x)
#define PT_STRINGIZE2(x) PT_WIDEN(#x)
#define PT_LINE_STRING PT_STRINGIZE(__LINE__)

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

/* Char version of __LINE__ macro, converts the numeral into a string */
#define PT_STRINGIZE_CHAR2(x) #x
#define PT_STRINGIZE_CHAR(x) PT_STRINGIZE_CHAR2(x)
#define PT_LINE_STRING_CHAR PT_STRINGIZE_CHAR(__LINE__)

/* Return codes for functions */
#define OKAY 0
#define NOT_OKAY_NO_BREAK 1 // Use this instead of NOT_OKAY when passed as parameter or assigned to a variable.

#if defined( _DEBUG ) && !defined( __ANDROID__ )
	#ifdef UNICODE
		static int ptDebugNotOkay(wchar_t *wcp_file, wchar_t *wcp_line)
		{
#if defined(_WIN32)
			if( IsDebuggerPresent() == 0 )
				MessageBoxW(NULL, wcp_file, wcp_line, MB_OK);
			else
				DebugBreak();
#else
			wprintf(L"Error: %ls, line: %ls\n", wcp_file, wcp_line);
#endif
			return(NOT_OKAY_NO_BREAK);
		}
		#define NOT_OKAY ptDebugNotOkay(PT_WFILE, PT_LINE_STRING)
	#else //UNICODE
		static int ptDebugNotOkay(char *cp_file, char *cp_line)
		{
#if defined(_WIN32)
			if( IsDebuggerPresent() == 0 )
				MessageBoxA(NULL, cp_file, cp_line, MB_OK);
			else
				DebugBreak();
#else
			printf("Error: %s, line: %s\n", cp_file, cp_line);
#endif
			return(NOT_OKAY_NO_BREAK);
		}
		#define NOT_OKAY ptDebugNotOkay(__FILE__, PT_LINE_STRING_CHAR)
	#endif //UNICODE

#elif !defined( __ANDROID__ ) //NOT DEBUG && WIN32
	#ifdef UNICODE
		static int ptReleaseNotOkay(wchar_t *wcp_file, wchar_t *wcp_line)
		{
#if defined(_WIN32)
			MessageBoxW(NULL, wcp_file, wcp_line, MB_OK);
#else
			wprintf(L"Error: %ls, line: %ls\n", wcp_file, wcp_line);
#endif
			return(NOT_OKAY_NO_BREAK);
		}
		#define NOT_OKAY ptReleaseNotOkay(PT_WFILE, PT_LINE_STRING)
	#else
		static int ptReleaseNotOkay(char *cp_file, char *cp_line)
		{
#if defined(_WIN32)
			MessageBoxA(NULL, cp_file, cp_line, MB_OK);
#else
			printf("Error: %s, line: %s\n", cp_file, cp_line);
#endif
			return(NOT_OKAY_NO_BREAK);
		}
		#define NOT_OKAY ptReleaseNotOkay( __FILE__, PT_LINE_STRING_CHAR)
	#endif //UNICODE

#else // NOT WIN32
	#define NOT_OKAY 1

#endif //_DEBUG && WIN32

/* Define a PT_HANDLE */
typedef int PT_HANDLE;

/* IS_TRUE, and IS_FALSE */
#define IS_FALSE 0
#define IS_TRUE  1 

#define realtype float

/* For setting the precision of cos/sin in response plotting functions */
#define respDouble realtype

/* OS Dependent defines */
#ifndef WIN32
	#ifndef HWND
		#define HWND void *
	#endif
#endif

// Support for Android debugging writes to Eclipse console
#ifdef __ANDROID__
	// Use of below requires LOCAL_LDLIBS := -llog in Android.mk
	#include <android/log.h>
	#define ANDROID_LOG_TAG __FILE__
	#define DPRINTF(...)  __android_log_print(ANDROID_LOG_DEBUG,ANDROID_LOG_TAG,__VA_ARGS__)
	#define IPRINTF(...)  __android_log_print(ANDROID_LOG_INFO,ANDROID_LOG_TAG,__VA_ARGS__)
	#define EPRINTF(...)  __android_log_print(ANDROID_LOG_ERROR,ANDROID_LOG_TAG,__VA_ARGS__)
#endif // __ANDROID__

// warning C4996: 'swprintf': function has been changed to conform with the ISO C standard, adding an extra character count parameter. To use the traditional Microsoft version, set _CRT_NON_CONFORMING_SWPRINTFS.
#define _CRT_NON_CONFORMING_SWPRINTFS

#endif //_CODEDEFS_H_
