#ifndef SCOPED_HANDLE_HPP
#define SCOPED_HANDLE_HPP

#include <windows.h>
#include<winsock.h>


// the class returns invalid handle for all handles which invalid handle is NULL
template<typename HandleType>
class NullHandleValue
{
public:
	static inline HandleType get() { return NULL; }
};

// the class returns invalid handle for all handles which invalid handle is INVALID_HANDLE_VALUE
class InvalidHandleValue
{
public:
	static inline HANDLE get() { return INVALID_HANDLE_VALUE; }
};

// the class returns invalid handle for all handles which invalid handle is INVALID_SOCKET
class InvalidSocketValue
{
public:
	static inline SOCKET get() { return INVALID_SOCKET; }
};

// the closeHandle  method
class CloseHandleMethod
{
public:
	static inline void close(HANDLE handle) { CloseHandle(handle); }
};

// the DisconnectNamedPipe method
class DisconnectNamedPipeMethod
{
public:
	static inline void close(HANDLE handle) { DisconnectNamedPipe(handle); }
};

// the delete object method
class DeleteObjectMethod
{
public:
	static inline void close(HGDIOBJ handle) { DeleteObject(handle); }
};

// the close socket method
class CloseSocketMethod
{
public:
	static inline void close(SOCKET handle) { closesocket(handle); }
};

// the unmap view of file method
class unMapViewOfFileMethod
{
public:
	static inline void close(LPVOID handle) { UnmapViewOfFile(handle); }
};

// the reg close key method
class RegCloseKeyMethod
{
public:
	static inline void close(HKEY handle) { RegCloseKey(handle); }
};

// the free library method
class FreeLibraryMethod
{
public:
	static inline void close(HMODULE handle) { FreeLibrary(handle); }
};

// the find close method
class FindCloseMethod
{
public:
	static inline void close(HANDLE handle) { FindClose(handle); }
};

template<typename HandleType, class InvalidValue, class CloseMethod>
class ScopedHandle
{
public:
	explicit ScopedHandle(HandleType handle = InvalidValue::get()) : 
		m_handle(handle) {
	}

	~ScopedHandle() {
		if ( m_handle != InvalidValue::get() )
			CloseMethod::close(m_handle);
	}

	void reset(HandleType handle = InvalidValue::get()) {
		ScopedHandle temp(m_handle);
		m_handle = handle;
	}

	void swap(ScopedHandle& other) {
		auto temp = m_handle;
		m_handle = other.m_handle;
		other.m_handle = temp;
	}

	HandleType get() {
		return m_handle;
	}

	explicit operator bool() const {
		return m_handle != InvalidValue::get();
	}

private:
	ScopedHandle(const ScopedHandle&) {}
	ScopedHandle& operator=(const ScopedHandle&) { return NULL; }

private:
	HandleType m_handle;
};


// wrapper for event handle returned from CreateEvent() method
typedef ScopedHandle<HANDLE, NullHandleValue<HANDLE>, CloseHandleMethod> EventScopedHandle;

// wrapper for thread handle returned from CreateThread() method
typedef ScopedHandle<HANDLE, NullHandleValue<HANDLE>, CloseHandleMethod> ThreadScopedHandle;

// wrapper for mapping file handle returne from CreateFileMapping() method
typedef ScopedHandle<HANDLE, NullHandleValue<HANDLE>, CloseHandleMethod> MappingScopedHandle;

// wrapper for the file mapping handle returned from CreateFile() method
typedef ScopedHandle<HANDLE, NullHandleValue<HANDLE>, CloseHandleMethod> FileScopedHandle;

// wrapper for the file mapping handle returned from CreateFile() method
typedef ScopedHandle<HANDLE, InvalidHandleValue, CloseHandleMethod> FileScopedHandleInv;

// wrapper for the file handle returnd from CreateNamedPipe
typedef ScopedHandle<HANDLE, InvalidHandleValue, DisconnectNamedPipeMethod> PipeScopedHandle;

// wrapper for the view of file handle returned from MapViewOfFile() method
typedef ScopedHandle<LPVOID, NullHandleValue<LPVOID>, unMapViewOfFileMethod> ViewOfFileScopedHandle;

// wrapper for the registry key handle returned from RegOpenKeyEx() method
typedef ScopedHandle<HKEY, NullHandleValue<HKEY>, RegCloseKeyMethod> RegKeyScopedHandle;

// wrapper for the module handle returned from LoadLibrary() method
typedef ScopedHandle<HMODULE, NullHandleValue<HMODULE>, FreeLibraryMethod> ModuleScopedHandle;

//  wrapper for the find file handle returned from FindFirstFile() method
typedef ScopedHandle<HANDLE, InvalidHandleValue, FindCloseMethod> FindFileScopedHandle;

// wrapper for socket handle returned from socket() method
typedef ScopedHandle<SOCKET, InvalidSocketValue, CloseSocketMethod> SocketScopedHandle;



#endif

