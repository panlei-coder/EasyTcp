//�ο����ӣ�https://blog.csdn.net/weixin_42299076/article/details/124828768
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
			printf("�˳�cmd_thread�߳�...\n");
			break;
		}
	}
}

class MyServer : public EasyTcpServer
{
public:

	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_joinCount++;
		//printf("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_joinCount--;
		//printf("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
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
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			pClient->SendData(&ret);
		}//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
		break;
		case CMD_LOGINOUT:
		{
			LoginOut* loginout = (LoginOut*)dh;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", pClient->Sockfd(), dh->dataLength);
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

	printf("���˳�...\n");
	while (true) {
	
	}
	return 0;
}