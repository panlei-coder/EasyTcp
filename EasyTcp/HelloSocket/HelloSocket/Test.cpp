#define WIN32_LEAN_AND_MEAN  //�����������ڵĿ⣬������ͻ

#include<windows.h>  //������WinSock1.h�Ŀ�
#include<WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")  //windows��̬���ӿ�
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);   //socket����ͨ��

	WSACleanup();
	return 0;
}