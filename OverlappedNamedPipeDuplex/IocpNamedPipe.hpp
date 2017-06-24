#ifndef IOCP_NAMED_PIPE_HPP
#define IOCP_NAMED_PIPE_HPP

#include <windows.h>
#include <memory>
#include <string>
#include "exports.h"
#include "OverlappedNamedPipeDuplexDll.h"

#pragma warning(push)
#pragma warning(disable: 4251)

namespace boost
{
	namespace interprocess
	{
		namespace overlapped
		{
			class COverlappedNamedPipeDuplexImp;

			class OV_NAMED_PIPE_DLL COverlappedNamedPipeDuplex
			{
			public:
				explicit COverlappedNamedPipeDuplex(std::string appName, OVNamedPipeDuplexServerCb cbServer, OVNamedPipeDuplexClientCb cbClient);
				virtual ~COverlappedNamedPipeDuplex();

			public:
				static std::string get_pipe_name();
				void startServer() const;
				void stopServer() const;
				void startClient() const;
				void stopClient() const;

			private:
				std::unique_ptr<COverlappedNamedPipeDuplexImp> m_pipe_comm_imp;
			};

		}
	}
}

#pragma warning(pop)

#endif