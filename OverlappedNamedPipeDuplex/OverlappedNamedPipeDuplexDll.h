#ifndef OVERLAPPEDNAMEDPIPEDUPLEXDLL_H
#define OVERLAPPEDNAMEDPIPEDUPLEXDLL_H

#include "exports.h"

#ifdef __cplusplus
#   define OVNPD_EXTERNC     extern "C"
#else
#   define OVNPD_EXTERNC
#endif // __cplusplus

#define OVNPD_CALLCONV		__stdcall

typedef void (OVNPD_CALLCONV *ov_named_pipe_duplex_server_cb)(void* pReceive);
typedef void (OVNPD_CALLCONV *ov_named_pipe_duplex_client_cb)(void* pReceive);

OVNPD_EXTERNC OV_NAMED_PIPE_DLL int nOverlappedNamedPipeDuplexDll;
OVNPD_EXTERNC int   OV_NAMED_PIPE_DLL  OVNPD_CALLCONV fnOverlappedNamedPipeDuplexDll(void);

OVNPD_EXTERNC void OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexGetPipeName(char* pszBuffer, size_t size);
OVNPD_EXTERNC int   OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexInit(char* pszAppName, ov_named_pipe_duplex_server_cb serverCb, ov_named_pipe_duplex_client_cb clientCb);
OVNPD_EXTERNC void OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexDeInit();

OVNPD_EXTERNC int   OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStartServer();
OVNPD_EXTERNC int   OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStopServer();
OVNPD_EXTERNC int   OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStartClient();
OVNPD_EXTERNC int   OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStopClient();


#endif