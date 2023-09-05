#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include "CELL.hpp"
#include"CELLServer.hpp"
#include "INetEvent.hpp"

class EasyTcpServer:public INetEvent
{
private:
	SOCKET _server_sock;
	std::vector<ClientSocket*> _clients;                 //���пͻ��˼���
	std::vector<CellServer*> _cellServers;              //���������ߣ��ӷ�����������
	CELLTimestamp _tTime;                             //�߾��ȼ�ʱ��
public:
	std::atomic_int _recvCount;                              //������Ϣ����
	std::atomic_int _joinCount;                              //����ͻ��˼���
	std::atomic_int _sendCount;                            //������Ϣ����
	std::atomic_int _msgCount;                             //��Ϣ�������
public:
	EasyTcpServer(){
		_server_sock = INVALID_SOCKET;
		_recvCount = 0;
		_joinCount = 0;
		_sendCount = 0;
		_msgCount = 0;
	}

	virtual ~EasyTcpServer(){
		_server_sock = INVALID_SOCKET;

		for (auto client : _clients) {
			delete client;
		}
		_clients.clear();
	}

	//��ʼ��socket
	int InitSocket(){
#ifdef _WIN32
		//����windows socket����
		WORD ver = MAKEWORD(2, 2);  //dll(��̬���ӿ�)�İ汾��
		WSADATA data;   //Ϊwindows���ص�socket��������
		WSAStartup(ver, &data);
#endif
		if (_server_sock != INVALID_SOCKET){
			printf("<socket=%d>�رվ�����...\n", (int)_server_sock);
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
			printf("����socket�ɹ�...\n");
		}
		return _server_sock;
	}

	//������˿�
	int Bind(const char* ip,unsigned short port){
		// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		/*
		    struct sockaddr_in{
		    sa_family_t     sin_family;   //��ַ�壨Address Family����Ҳ���ǵ�ַ����
		    uint16_t        sin_port;     //16λ�Ķ˿ں�
		    struct in_addr  sin_addr;     //32λIP��ַ
		    char            sin_zero[8];  //��ʹ�ã�һ����0���
		};
		htons�����ͱ����������ֽ�˳��ת��������ֽ�˳��(big-endian,��β��)��
		���������ڵ�ַ�ռ�洢��ʽ��Ϊ��λ�ֽڴ�����ڴ�ĵ͵�ַ����
		*/
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); // host to net unsigned short
#ifdef _WIN32
		if (ip){
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);//inet_addr("127.0.0.1")
		}
		else{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1")
		}
#else
		if (ip){
			_sin.sin_addr.s_addr = inet_addr(ip);//inet_addr("127.0.0.1")
		}
		else{
			_sin.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1")
		}
#endif

		//������������ͻ��https://www.cnblogs.com/jiguang321/p/10465012.html
		int ret = bind(_server_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret){
			printf("���󣬰�����˿�<%d>ʧ��...\n",port);
		}
		else{
			printf("������˿�<%d>�ɹ�...\n",port);
		}
		return ret;
	}

	//��������˿�
	int Listen(int backlog){
		// 3 listen ��������˿�
		int ret = listen(_server_sock, backlog);
		if (SOCKET_ERROR == ret){
			printf("socker=<%d>���󣬼�������˿�ʧ��...\n",_server_sock);
		}
		else {
			printf("socket=<%d>��������˿ڳɹ�...\n", _server_sock);
		}
		return ret;
	}
	
	//�ȴ��ͻ�������
	void Accept(){
		// 4 accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int  nAddrLen = sizeof(sockaddr_in);
		SOCKET client_sock = INVALID_SOCKET;
#ifdef _WIN32
		client_sock = accept(_server_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		client_sock = accept(_server_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif

		if (INVALID_SOCKET == client_sock){
			printf("���󣬽��յ���Ч�ͻ���socket...\n");
		}
		else{
			//SendDataToAll(client_sock);
			printf("�¿ͻ���<%d>���룺socket = %d,IP=%s\n", (const int)_joinCount,(int)client_sock, inet_ntoa(clientAddr.sin_addr));//inet_ntoa�����ֽ���IPת�����ʮ����IP
			addClientToCellServers(new ClientSocket(client_sock));
		}
	}

	void addClientToCellServers(ClientSocket* pClient){
		//_clients.push_back(pClient);
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers)	{
			if (pMinServer->getClientCount() > pCellServer->getClientCount()){
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);   //��ӵ����������
		OnNetJoin(pClient);
	}

	void Start(int cellserver_thread_count){
		for (int n = 0; n < cellserver_thread_count; n++){
			auto ser = new CellServer(n+1);
			_cellServers.push_back(ser);
			ser->setNetEventObj(this);
			ser->Start();
		}
	}

	//��ѯ������Ϣ
	void OnRun(){
		if (IsRun()){
			time4Msg();
			//������ socket�������֣�
			fd_set fdRead;   //�������ɶ��Ե�һ���ļ������� 
			fd_set fdWrite;  //��������д�Ե�һ���ļ�������
			fd_set fdExp;     //��������Ƿ����쳣�������ֵ��ļ������֣�ע�����󲻰������쳣�����ڣ�

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_server_sock, &fdRead);
			FD_SET(_server_sock, &fdWrite);
			FD_SET(_server_sock, &fdExp);

			//size_t(�޷�������)��Ŀ�����ṩһ�ֿ���ֲ�ķ�����������ϵͳ�п�Ѱַ���ڴ�����һ�µĳ���,ȡϵͳ���ܴﵽ�����ֵ
			//size_tΪ�޷�������n = n - 1 ==> n = n + (-1),��-1�ᱻת�����޷����������һ���ܴ�������ͻ����Խ��

			//nfds��һ������ֵ����ָfd_set������������������socket���ķ�Χ������������
			//���������ļ������������ֵ+1����windows���������������ν������д0���Ѿ�������ʵ����
			timeval t = { 0,1 };
			int ret = select(_server_sock + 1, &fdRead, &fdWrite, &fdExp, &t); //����ֵret<0�д���=0û�в�����>0�в���
			if (ret < 0){
				printf("select�������...\n");
				return;
			}
			//socket�пɶ������пͻ������ӽ���(���server_sock�Ƿ��ڼ�����)
			if (FD_ISSET(_server_sock, &fdRead)){
				FD_CLR(_server_sock, &fdRead); // ���������ֻ��������˼�����
				Accept();				
			}
		}
	}
	
	//�Ƿ�����
	bool IsRun(){
		return _server_sock != INVALID_SOCKET;
	}

	//�ر�����
	void Close(){
		//��һ���жϷ�ֹ��ε���
		if (_server_sock != INVALID_SOCKET){
			printf("EasyTcpServer%d:close begin\n", _server_sock);

#ifdef _WIN32
			closesocket(_server_sock);
			//--------------------------
			// ���windows socket����
			WSACleanup();
#else
			close(_server_sock);
#endif
			_server_sock = INVALID_SOCKET;
			_joinCount = 0;

			for (auto cellServer : _cellServers) {
				cellServer->Close();
				delete cellServer;
			}
			_cellServers.clear();

			printf("EasyTcpServer%d:close end\n", _server_sock);
		}
	}

	//��Ϣ��Ӧ
	void time4Msg(){
		auto t = _tTime.getElapsedSecond();
		if (t >= 1.0){
			printf("time<%lf>,socket<%d>,clients<%d>,recvCount<%d>,msgCount<%d>\n", t, _server_sock, (const int)_joinCount, (const int)(_recvCount /t),(const int)(_msgCount / t));
			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}

	virtual void OnNetJoin(ClientSocket* pClient){
		_joinCount++;
	}

	virtual void OnNetLeave(ClientSocket* pClient){
		_joinCount--;
	}

	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* dh){
		_msgCount++;
	}

	virtual void OnNetRecv(ClientSocket* pClient){
		_recvCount++;
	}
};
#endif // !_EasyTcpServer_hpp_