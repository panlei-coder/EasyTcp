#define WIN32_LEAN_AND_MEAN  //避免引入早期的库，发生冲突

#include<windows.h>  //包含了WinSock1.h的库
#include<WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")  //windows静态连接库
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);   //socket网络通信

	WSACleanup();
	return 0;
}