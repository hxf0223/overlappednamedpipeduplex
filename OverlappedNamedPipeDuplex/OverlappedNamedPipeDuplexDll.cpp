#include <memory>
#include "IocpNamedPipe.hpp"
#include "OverlappedNamedPipeDuplexDll.h"

using namespace boost::interprocess::overlapped;
static std::unique_ptr<overlappedNamedPipeDuplex> gOvNamedPipeDuplex;

OV_NAMED_PIPE_DLL int nOverlappedNamedPipeDuplexDll = 0;

OVNPD_EXTERNC int  OV_NAMED_PIPE_DLL OVNPD_CALLCONV fnOverlappedNamedPipeDuplexDll(void) {
	return 42;
}

OVNPD_EXTERNC void OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexGetPipeName(char* pszBuffer, size_t size) {
	auto name = overlappedNamedPipeDuplex::get_pipe_name();
	strcpy_s(pszBuffer, size, name.c_str());
}

OVNPD_EXTERNC int  OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexInit(char* pszAppName, OVNamedPipeDuplexServerCb serverCb, OVNamedPipeDuplexClientCb clientCb) {
	if ( !gOvNamedPipeDuplex ) {
		gOvNamedPipeDuplex = std::make_unique<overlappedNamedPipeDuplex>(std::string(pszAppName), serverCb, clientCb);
	}

	return 0;
}

OVNPD_EXTERNC void OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexDeInit() {
	if ( gOvNamedPipeDuplex ) gOvNamedPipeDuplex.reset();
}


OVNPD_EXTERNC int  OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStartServer() {
	if ( !gOvNamedPipeDuplex ) return -1;
	gOvNamedPipeDuplex->startServer();
	return 0;
}

OVNPD_EXTERNC int  OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStopServer() {
	if ( !gOvNamedPipeDuplex ) return -1;
	gOvNamedPipeDuplex->stopServer();
	return 0;
}

OVNPD_EXTERNC int  OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStartClient() {
	if ( !gOvNamedPipeDuplex ) return -1;
	gOvNamedPipeDuplex->startClient();
	return 0;
}

OVNPD_EXTERNC int  OV_NAMED_PIPE_DLL OVNPD_CALLCONV OverlappedNamedPipeDuplexStopClient() {
	if ( !gOvNamedPipeDuplex ) return -1;
	gOvNamedPipeDuplex->stopClient();
	return 0;
}

