#ifndef _INETEVENT_hpp_
#define _INETEVENT_hpp_

#include"CELL.hpp"
#include "CELLClient.hpp"

class CellServer;
//http://t.zoukankan.com/snowhumen-p-4097475.html
class INetEvent{  //委托（多态版中的单任务实现）
public:
	//纯虚函数
	//客户端离开事件
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	virtual void OnNetLeave(ClientSocket* pClient) = 0;
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* dh) = 0;
	virtual void OnNetRecv(ClientSocket* pClient) = 0;
};

#endif // !_INETEVENT_hpp_
