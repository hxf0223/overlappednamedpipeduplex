#ifndef OVERLAPPEDNAMEDPIPEDUPLEX_H
#define OVERLAPPEDNAMEDPIPEDUPLEX_H


#ifdef OV_NAMED_PIPE_EXPORT
#define OV_NAMED_PIPE_DLL	__declspec(dllexport)
#else
#define OV_NAMED_PIPE_DLL	__declspec(dllimport)
#endif

#endif


