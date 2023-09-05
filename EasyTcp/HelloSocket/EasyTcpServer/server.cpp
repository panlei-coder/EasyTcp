//参考链接：https://blog.csdn.net/weixin_42299076/article/details/124828768
#include<stdio.h>
#include<thread>
#include "EasyTcpServer.hpp"

bool g_run = true;
void cmdThread(){
	while (true){

		char order[128] = {};
		scanf("%s", order); //scanf_s("%s",order,128)

		if (0 == strcmp(order, "exit")){
			g_run = false;
			printf("退出cmd_thread线程...\n");
			break;
		}
	}
}

class MyServer : public EasyTcpServer
{
public:

	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_joinCount++;
		//printf("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_joinCount--;
		//printf("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* dh)
	{
		_msgCount++;
		switch (dh->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetHeartTime();
			//send recv 
			Login* login = (Login*)dh;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			LoginResult ret;
			pClient->SendData(&ret);
		}//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
		break;
		case CMD_LOGINOUT:
		{
			LoginOut* loginout = (LoginOut*)dh;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>收到未定义消息,数据长度：%d\n", pClient->Sockfd(), dh->dataLength);
			//DataHeader ret;
			//SendData(cSock, &ret);
		}
		break;
		}
	}

	virtual void OnNetRecv(ClientSocket* pClient)
	{
		_recvCount++;
		//printf("client<%d> leave\n", pClient->sockfd());
	}
private:

};

int main(){
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);

	std::thread th(cmdThread);
	th.detach();

	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);

	while (g_run){
		server.OnRun();
	}
	server.Close();

	printf("已退出...\n");
	while (true) {
	
	}
	return 0;
}
