#pragma  once

#ifdef ENABLE_TRACE
#include <crtdbg.h>

#pragma warning(disable:4996)
inline void _cdecl _trace(char* lpszFormat, ...) {
	va_list args;
	va_start(args, lpszFormat);

	char szBuffer[512];
	auto nBuf = _vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(WCHAR), lpszFormat, args);
	OutputDebugStringA(szBuffer);
	va_end(args);
}
#else
#  define _trace(char* lpszFormat, ...)
#endif
