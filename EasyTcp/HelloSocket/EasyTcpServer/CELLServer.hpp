#ifndef _CELLSERVER_hpp_
#define _CELLSERVER_hpp_

#include "INetEvent.hpp"

#include<vector>
#include<thread>
#include<mutex>

class CellServer{
private:
	int _id;
	bool _isRun = false;
	bool _wait = true;
	//正式客户队列
	std::vector<ClientSocket*> _clients;
	//缓冲客户队列
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	INetEvent* _pNetEvent;
	char _sz_Recv_Buff[RECV_BUFF_SIZE];  //缓冲区
	CellSendTask* _cellSendTask;

public:
	CellServer(int id){
		_id = id;
		_pNetEvent = nullptr;
		_cellSendTask = new CellSendTask();
	}

	~CellServer(){
		delete _cellSendTask;
		delete _pNetEvent;

		for (auto clientsBuff : _clientsBuff) {
			delete clientsBuff;
		}
		_clientsBuff.clear();
	}

	//event是EasyTcpServer类的当前对象（this）,所以它拥有EasyTcpServer的OnNetLeave方法
	void setNetEventObj(INetEvent* event){ //但这里做了强制类型转换，只拥有了OnNetLeave方法
		_pNetEvent = event;
	}

	//关闭连接
	void Close(){
		printf("CellServer%d:close begin\n", _id);

		_isRun = false;
		std::lock_guard<std::mutex> lock(_mutex);
		if (_wait) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
#ifdef _WIN32
		//清理客户端的套接字
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			closesocket(_clients[n]->Sockfd());
			delete _clients[n];
		}
#else
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			close(_clients[n]->Sockfd());
			delete _clients[n];
		}
#endif
		_clients.clear();

		printf("CellServer%d:close end\n", _id);
	}

	//查询网络消息
	fd_set _fdRead_bak;
	bool _clients_changed;
	SOCKET _maxSock;

	bool OnRun(){
		_clients_changed = true;
		while (_isRun){
			if (_clientsBuff.size() > 0)	{
				std::lock_guard<std::mutex> lg(_mutex);
				for (auto pClient : _clientsBuff){
					_clients.push_back(pClient);
				}
				_clientsBuff.clear(); //清理缓冲客户队列
				_clients_changed = true;
			}

			if (_clients.empty()){
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				_oldTime = CELLTime::getNowTimeInMilliSec();
				continue;
			}

			//伯克利 socket（描述字）
			fd_set fdRead;   //用来检查可读性的一组文件描述字 
			fd_set fdWrite;  //用来检查可写性的一组文件描述字
			fd_set fdExp;     //用来检查是否有异常条件出现的文件描述字（注：错误不包含在异常条件内）

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			/*FD_SET(_server_sock, &fdRead);
			FD_SET(_server_sock, &fdWrite);
			FD_SET(_server_sock, &fdExp);*/

			if (_clients_changed){
				//size_t(无符号整数)的目的是提供一种可移植的方法来声明与系统中可寻址的内存区域一致的长度,取系统所能达到的最大值
				//size_t为无符号数，n = n - 1 ==> n = n + (-1),而-1会被转化成无符号数，变成一个很大的数，就会出现越界
				_maxSock = _clients[0]->Sockfd();
				for (int n = (int)_clients.size() - 1; n >= 0; n--){
					FD_SET(_clients[n]->Sockfd(), &fdRead);
					if (_maxSock < _clients[n]->Sockfd()){
						_maxSock = _clients[n]->Sockfd();
					}
				}
				_clients_changed = false;
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			//nfds是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
			//即是所有文件描述符的最大值+1，在windows中这个参数（无所谓）可以写0，已经帮我们实现了
			//当select返回时,描述符集合中任何与没有准备好的描述符相对应的位都会被清0,所以,每次调用select时,
			//我们都得将所有描述符集合中关心的位置都置为1
			timeval t = { 0,0 };
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, &fdExp, &t); //返回值ret<0有错误，=0没有操作，>0有操作
			//int ret = select(_maxSock + 1, &fdRead, &fdWrite, &fdExp, nullptr);
			if (ret < 0){
				printf("select任务结束...\n");
				return false;
			}
			
			ReadData(fdRead);
			checkTime();
		}

		std::lock_guard<std::mutex> lock(_mutex);
		_wait = false;
	}

	time_t _oldTime = CELLTime::getNowTimeInMilliSec();
	void checkTime() {
		time_t nowTime = CELLTime::getNowTimeInMilliSec();
		time_t timeInterval = nowTime - _oldTime;
		_oldTime = nowTime;
		for (auto iter = _clients.begin(); iter != _clients.end();) {
			if ((*iter)->checkHeart(timeInterval)) {
				_clients_changed = true;
				if (_pNetEvent) {
					_pNetEvent->OnNetLeave(*iter);//生产者中的客户端信息删除
				}			
				closesocket((*iter)->Sockfd());
				delete (*iter);
				//iter删除存在的坑，erase之后会返回原本的下一个元素的位置给iter,如果不接收的话，iter会报错
				iter = _clients.erase(iter);    //消费者中的客户端信息删除，两者保持统一
				continue;
			}
			(*iter)->checkTiming(timeInterval);
			iter++;
		}
	}

	void ReadData(fd_set& fdRead) {
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			if (FD_ISSET(_clients[n]->Sockfd(), &fdRead)) {
				if (-1 == RecvData(_clients[n])) {
					auto iter = _clients.begin() + n;
					if (iter != _clients.end()) {
						_clients_changed = true;
						if (_pNetEvent) {
							_pNetEvent->OnNetLeave(_clients[n]);//生产者中的客户端信息删除
						}
						closesocket(_clients[n]->Sockfd());
						delete _clients[n];  //消费者中的客户端信息删除，两者保持统一
						_clients.erase(iter);
					}
				}
			}
		}
	}

	//接收消息 粘包/拆包
	int RecvData(ClientSocket* pClient){	
		int nlen = recv(pClient->Sockfd(), _sz_Recv_Buff, RECV_BUFF_SIZE, 0);  //返回接收到的数据长度	
		_pNetEvent->OnNetRecv(pClient);
		DataHeader* dh = (DataHeader*)_sz_Recv_Buff;
		// 6 处理请求
		if (nlen <= 0){
			printf("客户端<%d>已退出...\n", pClient->Sockfd());
			return -1;
		}
		memcpy(pClient->Msg_recv_Buff() + pClient->getLastPos(), _sz_Recv_Buff, nlen);
		pClient->setLastPos(pClient->getLastPos() + nlen);
		while (pClient->getLastPos() >= sizeof(DataHeader)){
			DataHeader* dh = (DataHeader*)pClient->Msg_recv_Buff();
			if (pClient->getLastPos() >= dh->dataLength){
				int nsize = pClient->getLastPos() - dh->dataLength;
				OnNetMsg(pClient, dh);
				memcpy(pClient->Msg_recv_Buff(), pClient->Msg_recv_Buff() + dh->dataLength, nsize);
				pClient->setLastPos(nsize);
			}
			else{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}

		return 1;
	}

	//消息响应
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* dh)
	{
		_pNetEvent->OnNetMsg(this, pClient, dh);
	}

	void addClient(ClientSocket* pClient){
		std::lock_guard<std::mutex> lg(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start(){
		_isRun = true;
		std::thread pThread(std::mem_fn(&CellServer::OnRun), this); //mem_fn将成员函数转化位函数对象，可以直接使用对象引用，指针，智能指针
		pThread.detach();
		//_cellSendTask->Start();
	}

	size_t getClientCount(){
		return _clients.size() + _clientsBuff.size();
	}

	void addTask(ClientSocket* pClient, DataHeader* dh){
		_cellSendTask->addCellTask([pClient, dh]() {
			pClient->SendData(dh);
			delete dh;
		});
	}
};

#endif // !_CELLSERVER_hpp_
