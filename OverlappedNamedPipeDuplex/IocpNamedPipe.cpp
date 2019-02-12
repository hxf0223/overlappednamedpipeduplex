#include <boost/foreach.hpp>
#include<boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/format.hpp>
#include<boost/algorithm/string.hpp>
#include <utility>

#include "scoped_handle.hpp"
#include "IocpNamedPipe.hpp"

//#define ENABLE_TRACE
#include "trace.h"

// https://github.com/RadicalZephyr/named-pipe
// http://www.cnblogs.com/cxq0017/p/6525027.html
// https://www.cppfans.org/1566.html
#pragma warning(disable: 4018)

// date time used by boost thread module
#define BOOST_DATE_TIME_SOURCE
#define BOOST_THREAD_NO_LIB

#define bforeach BOOST_FOREACH

//#define CLIENT_TEST_LOOP

using namespace std;

std::string& get_pipe_prefix() {
	static std::string prefix(R"(\\.\pipe\interprocess\)");
	return prefix;
}

// boost::regex_replace(str, boost::regex("[' ']{2,}"), " ");
inline std::string windowify_pipe_path(const std::string& str) {
	auto newstr = boost::replace_all_copy(str, "/", "\\");
	return newstr;
}

enum class PipeCommand : byte {
	request = 0x01,
	response = 0x10
};

#pragma pack(push)
#pragma pack(1)
struct PipeCommDataType {
	PipeCommand cmd;
	char name[256];
};
#pragma pack(pop)


namespace boost
{
	namespace interprocess
	{
		namespace overlapped
		{
			class overlappedNamedPipeDuplexImp {
			public:
				explicit overlappedNamedPipeDuplexImp(string appName, OVNamedPipeDuplexServerCb cbServer, OVNamedPipeDuplexClientCb cbClient);
				virtual ~overlappedNamedPipeDuplexImp();

			public:
				static std::string get_pipe_name();
				void startServer();
				void stopServer();
				void startClient();
				void stopClient();

			private:
				enum class pipeState : byte;
				struct PipeInstance;

				static void pipe_thread_client(HANDLE& hEvent, string& strAppName, OVNamedPipeDuplexClientCb clientCb);
				static void pipe_thread_server(HANDLE& hEvent, vector<PipeInstance>* pInstArray, OVNamedPipeDuplexServerCb serverCb);
				static boost::tuple<BOOL, pipeState> disconn_and_reconn_pipe(HANDLE hPipeInst, LPOVERLAPPED lpo);
				static BOOL conn_to_new_client(HANDLE hPipe, LPOVERLAPPED lpo);
				static void disconnect_pipe(HANDLE handle);
				static void disconnect_pipes(vector<HANDLE>* hs);
				static void close_events(vector<HANDLE>* hs);
				static void close_event(HANDLE h);

			private:
				static const std::string sm_pipe_name;
				static const int sm_pipe_create_buffer_size_input;
				static const int sm_pipe_create_buffer_size_output;
				static const int sm_pipe_ins_num;

				std::string m_appname;
				std::unique_ptr<boost::thread> m_thread_client;
				std::unique_ptr<boost::thread> m_thread_server;
				std::unique_ptr<std::vector<PipeInstance>> m_pis;
				std::unique_ptr<void, std::function<void(HANDLE)>> m_hClientExitEvent;
				std::unique_ptr<void, std::function<void(HANDLE)>> m_hServerExitEvent;

				//std::unique_ptr<std::vector<HANDLE>, std::function<void(std::vector<HANDLE>*)>> m_hEvents;

				OVNamedPipeDuplexServerCb m_servercb;
				OVNamedPipeDuplexClientCb m_clientcb;
			};

			enum class overlappedNamedPipeDuplexImp::pipeState : byte {
				connecting_state = 0,		// init state, should be 0
				reading_state = 1,
				writing_state = 2
			};

			struct overlappedNamedPipeDuplexImp::PipeInstance {
				OVERLAPPED overlap;
				HANDLE hPipeInst;

				PipeCommDataType reqData;
				DWORD cbRead;

				PipeCommDataType respData;
				DWORD cbToWrite;

				pipeState dwState;
				BOOL fPendingIO;
			};


			const std::string overlappedNamedPipeDuplexImp::sm_pipe_name = get_pipe_prefix() + windowify_pipe_path("udsAppMutex");
			const int overlappedNamedPipeDuplexImp::sm_pipe_create_buffer_size_input = 1024;
			const int overlappedNamedPipeDuplexImp::sm_pipe_create_buffer_size_output = 1024;
			const int overlappedNamedPipeDuplexImp::sm_pipe_ins_num = 5;

			/**
			 * \brief
			 * \param appName
			 * \param cbServer
			 * \param cbClient
			 */
			overlappedNamedPipeDuplexImp::overlappedNamedPipeDuplexImp(string appName, OVNamedPipeDuplexServerCb cbServer, OVNamedPipeDuplexClientCb cbClient) :
				m_appname(std::move(appName)),
				m_pis(new std::vector<PipeInstance>(sm_pipe_ins_num)),
				m_hClientExitEvent(CreateEvent(nullptr, TRUE, FALSE, "ClientExitEvent"), close_event),
				m_hServerExitEvent(CreateEvent(nullptr, TRUE, FALSE, "ServerExitEvent"), close_event),
				m_servercb(cbServer), m_clientcb(cbClient) {
				bforeach(auto& x, *m_pis) {
					memset(&x, 0, sizeof x);
					strcpy_s(x.respData.name, sizeof x.respData.name, m_appname.c_str());
				}
			}

			overlappedNamedPipeDuplexImp::~overlappedNamedPipeDuplexImp() {
				stopServer();
				stopClient();
			}

			std::string overlappedNamedPipeDuplexImp::get_pipe_name() {
				return sm_pipe_name;
			}

			void overlappedNamedPipeDuplexImp::startServer() {
				if (m_thread_server) {
					SetEvent(m_hServerExitEvent.get());
					m_thread_server->join();
					m_thread_server.reset();
				}

				auto bsuccess = ResetEvent(m_hServerExitEvent.get());
				// m_thread_server = std::make_unique<boost::thread>(COverlappedNamedPipeDuplexImp::pipe_thread_server, 
				// this, m_hEvents.get(), m_pis.get(), m_servercb);		// thread func is member func
				m_thread_server = std::make_unique<boost::thread>(pipe_thread_server,
					m_hServerExitEvent.get(), m_pis.get(), m_servercb);
			}

			void overlappedNamedPipeDuplexImp::stopServer() {
				if (m_thread_server) {
					SetEvent(m_hServerExitEvent.get());
					m_thread_server->join();
					m_thread_server.reset();
				}
			}

			void overlappedNamedPipeDuplexImp::startClient() {
				if (m_thread_client) {
					SetEvent(m_hClientExitEvent.get());
					m_thread_client->join();
					m_thread_client.reset();
				}

				auto bsuccess = ResetEvent(m_hClientExitEvent.get());
				m_thread_client = std::make_unique<boost::thread>(pipe_thread_client,
					m_hClientExitEvent.get(), m_appname, m_clientcb);
			}

			void overlappedNamedPipeDuplexImp::stopClient() {
				if (m_thread_client) {
					SetEvent(m_hClientExitEvent.get());
					m_thread_client->join();
					m_thread_client.reset();
				}
			}

			void overlappedNamedPipeDuplexImp::pipe_thread_client(HANDLE& hEvent, std::string& strAppName, OVNamedPipeDuplexClientCb clientCb) {
				_trace("pipe_thread_client started, thread id: 0x%llx\n", boost::this_thread::get_id());
				PipeCommDataType resp_data{}, request_data = { PipeCommand::request, 0 };
				strcpy_s(request_data.name, sizeof request_data.name, strAppName.c_str());
				auto hPipe = InvalidHandleValue::get();

				BOOL fSuccess;
				DWORD cb_read, cb_written;

				const auto dw_wait = WaitForSingleObject(hEvent, 0);
				while (dw_wait != WAIT_OBJECT_0) {
					hPipe = CreateFile(sm_pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
					if (hPipe != INVALID_HANDLE_VALUE) break;

					// Exit if an error other than ERROR_PIPE_BUSY occurs. 
					if (GetLastError() != ERROR_PIPE_BUSY) {
						_trace("pipe_thread_client Could not open pipe. GLE=%d\n", GetLastError());
						return;
					}

					// All pipe instances are busy, so wait 
					if (!WaitNamedPipe(sm_pipe_name.c_str(), 100)) {
						_trace("pipe_thread_client WaitNamedPipe timed out.\n");
						return;
					}
				}

				FileScopedHandleInv fshPipe(hPipe);

				// The pipe connected; change to message-read mode. 
				DWORD dwMode = PIPE_READMODE_MESSAGE;
				if (!SetNamedPipeHandleState(hPipe, &dwMode/*new pipe mode*/, nullptr, nullptr)) {
					_trace("pipe_thread_client SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
					return;
				}

#ifdef CLIENT_TEST_LOOP
				dwWait = WaitForSingleObject(hEvent, 0);
				while (dwWait != WAIT_OBJECT_0) {
#endif
					// Send a message to the pipe server. 
					DWORD cbToWrite = sizeof request_data;
					_trace("pipe_thread_client will WriteFile, name: %s\n", request_data.name);
					if (!WriteFile(hPipe, &request_data, cbToWrite, &cb_written, nullptr)) {
						_trace("pipe_thread_client WriteFile to pipe failed. GLE=%d\n", GetLastError());
						return;
					}

					do {
						fSuccess = ReadFile(hPipe, &resp_data, sizeof resp_data, &cb_read, nullptr);   // not overlapped 
						if (!fSuccess && GetLastError() != ERROR_MORE_DATA) break;
						_trace("pipe_thread_client ReadFile, num %d, data: %s\n", cb_read, resp_data.name);
						if (fSuccess && clientCb) clientCb(&resp_data);
					} while (!fSuccess);

					if (!fSuccess) { _trace("read response from server fail\n"); }
#ifdef CLIENT_TEST_LOOP
				}
#endif

				_trace("pipe_thread_client exit\n");
			}

#pragma region "Server thead: pipe_thread_server"

			void overlappedNamedPipeDuplexImp::pipe_thread_server(HANDLE& hEvent, std::vector<PipeInstance>* pInstArray, OVNamedPipeDuplexServerCb serverCb) {
				_trace("pipe_thread_server started, thread id: 0x%llx\n", boost::this_thread::get_id());
				const int pipe_ins_size = pInstArray->size();
				std::shared_ptr<std::vector<HANDLE>> sp_hpipes(new std::vector<HANDLE>(pipe_ins_size), disconnect_pipes);
				bforeach(auto& x, *sp_hpipes) x = InvalidHandleValue::get();

				std::vector<HANDLE> hall_events(pipe_ins_size + 1);
				std::shared_ptr<std::vector<HANDLE>> sp_wait_events(new std::vector<HANDLE>(pipe_ins_size), close_events);
				for (auto i = 0; i < pipe_ins_size; i++) {
					auto temp = boost::str(boost::format("%s_%02d") % "ServerOVEvent" %i);
					hall_events[i] = sp_wait_events->at(i) = CreateEvent(nullptr, TRUE, TRUE, temp.c_str());
				}

				hall_events.at(pipe_ins_size) = hEvent;

				auto pinsarray = pInstArray->data();
				const auto phppipes = sp_hpipes->data();

				for (auto i = 0; i < pipe_ins_size; i++) {
					phppipes[i] = pinsarray[i].hPipeInst = CreateNamedPipe(
						sm_pipe_name.c_str(),
						PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
						PIPE_TYPE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
						pipe_ins_size, sm_pipe_create_buffer_size_input, sm_pipe_create_buffer_size_output, 200, nullptr);
					if (InvalidHandleValue::get() == phppipes[i]) {
						_trace("CreateNamedPipe failed with %d.\n", GetLastError());
						return;
					}

					pinsarray[i].respData.cmd = PipeCommand::response;
					pinsarray[i].overlap.hEvent = hall_events[i];
					pinsarray[i].cbToWrite = sizeof pinsarray[i].respData;
					pinsarray[i].fPendingIO = conn_to_new_client(pinsarray[i].hPipeInst, &pinsarray[i].overlap);
					pinsarray[i].dwState = (pinsarray[i].fPendingIO) ? pipeState::connecting_state : pipeState::reading_state;
				}

				BOOL fSuccess;
				DWORD  cbRet;

				while (true) {
					const auto dw_wait = WaitForMultipleObjects(pipe_ins_size + 1, &(hall_events[0]), FALSE, INFINITE);
					auto waitid = dw_wait - WAIT_OBJECT_0;
					if (waitid < 0 || waitid >= pipe_ins_size) {
						_trace("pipe_thread_server exit, dwWait %ld\n", dw_wait);
						return;
					}

					if (pinsarray[waitid].fPendingIO) {
						fSuccess = GetOverlappedResult(pinsarray[waitid].hPipeInst, &(pinsarray[waitid].overlap), &cbRet, FALSE);
						switch (pinsarray[waitid].dwState) {
						case pipeState::connecting_state:
							if (!fSuccess) {
								_trace("pipe_thread_server GetOverlappedResult error %d.\n", GetLastError());
								return;
							}
							pinsarray[waitid].dwState = pipeState::reading_state;
							break;

						case pipeState::reading_state:		// Pending read operation 
							if (!fSuccess || cbRet == 0) {
								boost::tie(pinsarray[waitid].fPendingIO, pinsarray[waitid].dwState) =
									disconn_and_reconn_pipe(pinsarray[waitid].hPipeInst, &(pinsarray[waitid].overlap));
								_trace("pipe_thread_server READING_STATE disconn_and_reconn_pipe\n");
								continue;
							}
							pinsarray[waitid].cbRead = cbRet;
							pinsarray[waitid].dwState = pipeState::writing_state;
							_trace("pipe_thread_server ReadFile<1> OK, num %ld: %s\n", cbRet, pinsarray[waitid].reqData.name);
							if (serverCb) serverCb(&pinsarray[waitid].reqData);
							break;

						case pipeState::writing_state:		// Pending write operation 
							if (!fSuccess || cbRet != pinsarray[waitid].cbToWrite) {
								boost::tie(pinsarray[waitid].fPendingIO, pinsarray[waitid].dwState) =
									disconn_and_reconn_pipe(pinsarray[waitid].hPipeInst, &(pinsarray[waitid].overlap));
								_trace("pipe_thread_server WRITING_STATE disconn_and_reconn_pipe\n");
								continue;
							}
							pinsarray[waitid].dwState = pipeState::reading_state;
							break;

						default:
							_trace("pipe_thread_server Invalid pipe state.\n");
							return;
						}
					}

					switch (pinsarray[waitid].dwState) {
						// READING_STATE: 
						// The pipe instance is connected to the client 
						// and is ready to read a request from the client. 
					case pipeState::reading_state:
						fSuccess = ReadFile(pinsarray[waitid].hPipeInst,
							&pinsarray[waitid].reqData,
							sizeof pinsarray[waitid].reqData,
							&pinsarray[waitid].cbRead, &pinsarray[waitid].overlap);

						if (fSuccess && pinsarray[waitid].cbRead != 0) {		// The read operation completed successfully. 
							pinsarray[waitid].fPendingIO = FALSE;
							pinsarray[waitid].dwState = pipeState::writing_state;
							_trace("pipe_thread_server ReadFile<2> OK, num %ld: %s\n",
								pinsarray[waitid].cbRead, pinsarray[waitid].reqData.name);
							if (serverCb) serverCb(&pinsarray[waitid].reqData);
							continue;
						}

						// The read operation is still pending. 
						if (!fSuccess && (GetLastError() == ERROR_IO_PENDING)) {
							pinsarray[waitid].fPendingIO = TRUE;
							_trace("pipe_thread_server ReadFile IO pending\n");
							continue;
						}

						// An error occurred; disconnect from the client. 
						boost::tie(pinsarray[waitid].fPendingIO, pinsarray[waitid].dwState) =
							disconn_and_reconn_pipe(pinsarray[waitid].hPipeInst, &pinsarray[waitid].overlap);
						_trace("pipe_thread_server ReadFile fail, disconn_and_reconn_pipe\n");
						break;

						// WRITING_STATE: 
						// The request was successfully read from the client. 
						// Get the reply data and write it to the client. 
					case pipeState::writing_state:
						fSuccess = WriteFile(pinsarray[waitid].hPipeInst,
							&pinsarray[waitid].respData,
							sizeof pinsarray[waitid].respData,
							&cbRet, &pinsarray[waitid].overlap);

						// The write operation completed successfully. 
						if (fSuccess && cbRet == pinsarray[waitid].cbToWrite) {
							pinsarray[waitid].fPendingIO = FALSE;
							pinsarray[waitid].dwState = pipeState::reading_state;
							_trace("pipe_thread_server WriteFile OK, data: %s\n", pinsarray[waitid].respData.name);
							continue;
						}

						// The write operation is still pending. 
						if (!fSuccess && (GetLastError() == ERROR_IO_PENDING)) {
							pinsarray[waitid].fPendingIO = TRUE;
							_trace("pipe_thread_server WriteFile IO pending\n");
							continue;
						}

						// An error occurred; disconnect from the client. 
						boost::tie(pinsarray[waitid].fPendingIO, pinsarray[waitid].dwState) =
							disconn_and_reconn_pipe(pinsarray[waitid].hPipeInst, &pinsarray[waitid].overlap);
						_trace("pipe_thread_server WriteFile fail, disconn_and_reconn_pipe\n");
						break;

					default:
						_trace("pipe_thread_server Invalid pipe state.\n");
						return;
					}

				}

				_trace("pipe_thread_server exit/n");
			}

#pragma endregion

			void overlappedNamedPipeDuplexImp::disconnect_pipe(HANDLE handle) {
				PipeScopedHandle psh(handle);
			}

			void overlappedNamedPipeDuplexImp::disconnect_pipes(std::vector<HANDLE>* hs) {
				bforeach(auto& x, *hs) {
					PipeScopedHandle psh(x);
				}
				delete hs;
			}

			void overlappedNamedPipeDuplexImp::close_events(std::vector<HANDLE>* hs) {
				bforeach(auto& x, *hs) {
					EventScopedHandle eh(x);
				}
				delete hs;
			}

			void overlappedNamedPipeDuplexImp::close_event(HANDLE h) {
				EventScopedHandle eh(h);
			}

			boost::tuple<BOOL, overlappedNamedPipeDuplexImp::pipeState> overlappedNamedPipeDuplexImp::disconn_and_reconn_pipe(HANDLE hPipeInst, LPOVERLAPPED lpo) {
				if (!DisconnectNamedPipe(hPipeInst)) {
					_trace("DisconnectNamedPipe failed with %d.\n", GetLastError());
				}

				// Call a subroutine to connect to the new client. 
				auto fPendingIO = conn_to_new_client(hPipeInst, lpo);
				auto dwState = fPendingIO ? pipeState::connecting_state : pipeState::reading_state;
				return boost::make_tuple(fPendingIO, dwState);
			}

			BOOL overlappedNamedPipeDuplexImp::conn_to_new_client(HANDLE hPipe, LPOVERLAPPED lpo) {
				const auto connected = ConnectNamedPipe(hPipe, lpo);
				if (connected) {	// Overlapped ConnectNamedPipe should return zero. 
					_trace("ConnectNamedPipe failed with %d.\n", GetLastError());
					return FALSE;
				}

				auto fPendingIO = FALSE;
				switch (GetLastError()) {
				case ERROR_IO_PENDING:
					fPendingIO = TRUE;
					break;

				case ERROR_PIPE_CONNECTED:		// Client is already connected, so signal an event. 
					if (SetEvent(lpo->hEvent))
						break;

				default:		// If an error occurs during the connect operation... 
					printf("ConnectNamedPipe failed with %d.\n", GetLastError());
					return FALSE;
				}

				return fPendingIO;
			}

			overlappedNamedPipeDuplex::overlappedNamedPipeDuplex(std::string appName, OVNamedPipeDuplexServerCb cbServer, OVNamedPipeDuplexClientCb cbClient) :
				m_pipe_comm_imp(new overlappedNamedPipeDuplexImp(appName, cbServer, cbClient)) {
			}

			overlappedNamedPipeDuplex::~overlappedNamedPipeDuplex() = default;

			std::string overlappedNamedPipeDuplex::get_pipe_name() {
				return overlappedNamedPipeDuplexImp::get_pipe_name();
			}

			void overlappedNamedPipeDuplex::startServer() const {
				m_pipe_comm_imp->startServer();
			}

			void overlappedNamedPipeDuplex::stopServer() const {
				m_pipe_comm_imp->stopServer();
			}

			void overlappedNamedPipeDuplex::startClient() const {
				m_pipe_comm_imp->startClient();
			}

			void overlappedNamedPipeDuplex::stopClient() const {
				m_pipe_comm_imp->stopClient();
			}

		}
	}
}



