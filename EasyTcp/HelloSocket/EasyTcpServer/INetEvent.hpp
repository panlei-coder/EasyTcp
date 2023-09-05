#ifndef _INETEVENT_hpp_
#define _INETEVENT_hpp_

#include"CELL.hpp"
#include "CELLClient.hpp"

class CellServer;
//http://t.zoukankan.com/snowhumen-p-4097475.html
class INetEvent{  //ί�У���̬���еĵ�����ʵ�֣�
public:
	//���麯��
	//�ͻ����뿪�¼�
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	virtual void OnNetLeave(ClientSocket* pClient) = 0;
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* dh) = 0;
	virtual void OnNetRecv(ClientSocket* pClient) = 0;
};

#endif // !_INETEVENT_hpp_
