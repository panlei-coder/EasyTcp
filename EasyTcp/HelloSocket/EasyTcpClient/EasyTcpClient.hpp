#ifndef  _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
	//#define FD_SETSIZE 1024
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS  //ȥ��warning
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h>
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include "MessageHeader.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 1024
#endif // !RECV_BUFF_SZIE

class EasyTcpClient{
	SOCKET _server_sock;
	char _sz_Recv_Buff[RECV_BUFF_SIZE] = {};		//��Ϣ������
	char _msg_Recv_Buff[RECV_BUFF_SIZE * 10] = {};       //�ڶ�������
	int _last_pos = 0; //�ڶ���������ĩβλ��
public:
	EasyTcpClient(){
		_server_sock = INVALID_SOCKET;
	}

	//�����������ܹ���ɾ��ָ����������ָ��ʱ���ܹ�������������������������ڴ��ͷţ������ڴ�й¶
	virtual ~EasyTcpClient(){
		Close();
	}

	int InitSocket(){
#ifdef _WIN32
		//����windows socket����
		WORD ver = MAKEWORD(2, 2);  //dll(��̬���ӿ�)�İ汾��
		WSADATA data;   //Ϊwindows���ص�socket��������
		WSAStartup(ver, &data);
#endif
		if (_server_sock != INVALID_SOCKET){
			printf("<socket=%d>�رվ�����...\n", _server_sock);
			Close();
		}

		//-----------------------
		// ��socket api�������׵�tcp�ͻ���
		// 1 ����һ��socket
		_server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _server_sock){
			printf("���󣬽���socketʧ��...\n");
		}
		else {
			printf("����socket=<%d>�ɹ�...\n",_server_sock);
		}
		return _server_sock;
	}

	//���ӷ�����
	int Connect(const char* ip, unsigned short port){
		//�������Ч��socket����Ҫ���³�ʼ��
		if (INVALID_SOCKET == _server_sock){
			InitSocket();
		}

		// 2 ���ӷ����� connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //���ӵ�����˵Ķ˿ں� (1024-65536)

#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip); //ָ����Ч�ķ���˵�ip
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		printf("<socket=%d>�������ӷ�����<%s:%d>...\n", _server_sock, ip, port);
		int ret = connect(_server_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret){
			printf("<socket=%d>�������ӷ�����<%s:%d>.ʧ��...\n", _server_sock, ip, port);
		}
		else {
			printf("<socket=%d>���ӷ������ɹ�<%s:%d>...\n", _server_sock, ip, port);
		}
		return ret;
	}

	//�ر�socket
	void Close(){
		//��һ���жϷ�ֹ��ε���
		if (_server_sock != INVALID_SOCKET){
#ifdef _WIN32
			// 7 �ر��׽��� closesocket
			closesocket(_server_sock); // �ر��׽���ʱ�����˷���һ����Ϣ����֪�˳�
			//----------------------- 
			//���windows socket����
			WSACleanup();
#else
			close(_server_sock);
#endif
			_server_sock = INVALID_SOCKET;
		}
	}

	//��ѯ������Ϣ
	void OnRun(){
		if (IsRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_server_sock, &fdReads);

			timeval t = { 0,0 };
			int ret = select(_server_sock + 1, &fdReads, 0, 0, &t);
			//int ret = select(_server_sock + 1, &fdReads, 0, 0, nullptr);
			if (ret < 0)
			{
				printf("<socket=%d>select�������1...\n", _server_sock);
				Close();
				return;
			}
			if (FD_ISSET(_server_sock, &fdReads))
			{
				FD_CLR(_server_sock, &fdReads); // ���������ֻ���������˼�����
				if (-1 == RecvData())
				{
					printf("<socket=%d>select�����ѽ���2...\n", _server_sock);
					Close();
					return;
				}
			}
		}
	}

	//�Ƿ�����
	bool IsRun(){
		return _server_sock != INVALID_SOCKET;
	}

	//�������� ����ճ��
	int RecvData(){
		int remain_len = 0;   //�ڶ�������ʣ����Ϣ�ĳ���
		// 5 ���տͻ�������
		int nlen = recv(_server_sock, _sz_Recv_Buff, RECV_BUFF_SIZE, 0);  //����ʵ�ʽ��յ������ݳ���
		if (nlen <= 0){
			printf("�������<socket=%d>�Ͽ����ӣ���������...\n",_server_sock);
			return -1;
		}

		memcpy(_msg_Recv_Buff + _last_pos, _sz_Recv_Buff, nlen);
		_last_pos += nlen;
		
		while (_last_pos >= sizeof(DataHeader)){//�����������ֱ�ӽ�����һ����Ϣ���չ���
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* dh = (DataHeader*)_msg_Recv_Buff;
			//�ж���Ϣ�����������ݳ����Ƿ���ڵ���������Ϣ��ĳ���
			if (_last_pos >= dh->dataLength){
				//�õ���Ϣ������ʣ��δ�������ݵĳ���
				remain_len = _last_pos - dh->dataLength;
				//����������Ϣ
				OnNetMsg(dh);   //����������ָ�룬ָ�������Ϣ����ڴ�飬�����ݳ���Ӧ���Ǵ��ڵ���������Ϣ��ĳ���
				//����Ϣ������ʣ���δ��������ǰ��
				memcpy(_msg_Recv_Buff, _msg_Recv_Buff + dh->dataLength, remain_len);
				//��Ϣ������������β��λ��ǰ��
				_last_pos = remain_len;
			}
			else{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 1;
	}

	//��Ϣ��Ӧ
	virtual void OnNetMsg(DataHeader* dh){
		switch (dh->cmd){
			case CMD_LOGIN_RESULT:{
				// 6 ���շ������Ϣ recv
				LoginResult* login_ret = (LoginResult*)dh;
				printf("<socket=%d>�յ��������Ϣ��CMD_LOGIN_RESULT,���ݳ���Ϊ��%d\n",_server_sock, login_ret->dataLength);
			}break;
			case CMD_LOGINOUT_RESULT:{
				// 6 ���շ������Ϣ recv
				LoginOutResult* logout_ret = (LoginOutResult*)dh;
				//printf("<socket=%d>�յ��������Ϣ��CMD_LOGINOUT_RESULT,���ݳ���Ϊ��%d\n", _server_sock, logout_ret->dataLength);
			}break;
			case CMD_NEW_USER_JOIN:{
				// 6 ���շ������Ϣ recv
				NewUserJoin* newUserJoin = (NewUserJoin*)dh;
				//printf("�¿ͻ���<%d>���룺CMD_NEW_USER_JOIN,���ݳ���Ϊ��%d\n", newUserJoin->sock, newUserJoin->dataLength);
			}break;
			default:{
				printf("<socket=%d>�յ������δ�������Ϣ�����ݳ���Ϊ��%d\n", _server_sock, dh->dataLength);
			}break;
		}
	}

	//��������
	int SendData(DataHeader* dh,int length){
		if (IsRun() && dh){
			return send(_server_sock, (const char*)dh, length, 0);
		}
		return SOCKET_ERROR;
	}

private:

};
#endif // ! _EasyTcpClient_hpp_