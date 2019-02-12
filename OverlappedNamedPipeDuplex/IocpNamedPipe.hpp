#ifndef IOCP_NAMED_PIPE_HPP
#define IOCP_NAMED_PIPE_HPP

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
			class overlappedNamedPipeDuplexImp;

			class OV_NAMED_PIPE_DLL overlappedNamedPipeDuplex final {
			public:
				explicit overlappedNamedPipeDuplex(std::string appName, ov_named_pipe_duplex_server_cb cbServer, ov_named_pipe_duplex_client_cb cbClient);
				~overlappedNamedPipeDuplex();

				overlappedNamedPipeDuplex(const overlappedNamedPipeDuplex&) = delete;
				overlappedNamedPipeDuplex& operator=(const overlappedNamedPipeDuplex&) = delete;
				overlappedNamedPipeDuplex(const overlappedNamedPipeDuplex&&) = delete;
				overlappedNamedPipeDuplex& operator=(const overlappedNamedPipeDuplex&&) = delete;

			public:
				static std::string get_pipe_name();
				void startServer() const;
				void stopServer() const;
				void startClient() const;
				void stopClient() const;

			private:
				std::unique_ptr<overlappedNamedPipeDuplexImp> m_pipe_comm_imp;
			};

		}
	}
}

#pragma warning(pop)

#endif