#ifndef OVERLAPPEDNAMEDPIPEDUPLEXDLL_H
#define OVERLAPPEDNAMEDPIPEDUPLEXDLL_H

#include "exports.h"

#ifdef __cplusplus
#   define OVNPD_EXTERNC     extern "C"
#else
#   define OVNPD_EXTERNC
#endif // __cplusplus

#define OVNPD_CALLCONV		__stdcall

typedef void (*OVNamedPipeDuplexServerCb)(char* pszReceive);
typedef void(*OVNamedPipeDuplexClientCb)(char* pszReceive);

OVNPD_EXTERNC OV_NAMED_PIPE_DLL int nOverlappedNamedPipeDuplexDll;
OVNPD_EXTERNC OV_NAMED_PIPE_DLL int OVNPD_CALLCONV fnOverlappedNamedPipeDuplexDll(void);

OVNPD_EXTERNC OV_NAMED_PIPE_DLL void OVNPD_CALLCONV OverlappedNamedPipeDuplexGetPipeName(char* pszBuffer, size_t size);
OVNPD_EXTERNC OV_NAMED_PIPE_DLL int OVNPD_CALLCONV OverlappedNamedPipeDuplexInit(char* pszAppName, OVNamedPipeDuplexServerCb serverCb, OVNamedPipeDuplexClientCb clientCb);
OVNPD_EXTERNC OV_NAMED_PIPE_DLL int OVNPD_CALLCONV OverlappedNamedPipeDuplexStartServer();
OVNPD_EXTERNC OV_NAMED_PIPE_DLL int OVNPD_CALLCONV OverlappedNamedPipeDuplexStopServer();
OVNPD_EXTERNC OV_NAMED_PIPE_DLL int OVNPD_CALLCONV OverlappedNamedPipeDuplexStartClient();
OVNPD_EXTERNC OV_NAMED_PIPE_DLL int OVNPD_CALLCONV OverlappedNamedPipeDuplexStopClient();


#endif