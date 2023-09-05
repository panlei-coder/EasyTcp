#ifndef _CELL_hpp_
#define _CELL_hpp_
 
#ifdef _WIN32
	#define FD_SETSIZE 10240
	#define WIN32_LEAN_AND_MEAN  //避免早期的代码库冲突
	#define _WINSOCK_DEPRECATED_NO_WARNINGS  //去掉warning
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")  //添加windows socket的库路径		
#else
	#include<unistd.h> // uni std
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR           (-1)
#endif

#include<stdio.h>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CELLSendTask.hpp"

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024
	#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif // !RECV_BUFF_SZIE

#define _CellServer_Thread_Count 4

#endif // !_CELL_hpp_
