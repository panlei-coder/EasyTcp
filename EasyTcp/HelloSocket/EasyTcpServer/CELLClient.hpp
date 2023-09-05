#ifndef _CELLCLIENT_hpp_
#define _CELLCLIENT_hpp_

#include "CELL.hpp"

#define MAX_HEARTTIMEOUT 60000
#define MAX_TIMINGOUT 200

class ClientSocket{
private:
	SOCKET _sockfd;
	char _msg_Recv_Buff[RECV_BUFF_SIZE*5];	//第二缓冲区
	int _last_recv_pos;				//第二缓冲区的末尾位置

	char _msg_Send_Buff[SEND_BUFF_SIZE];     //;第二缓冲区
	int _last_send_pos;			//第二缓冲区的末尾位置

	time_t _heartTime;
	time_t _timing;

public:
	ClientSocket(SOCKET sock = INVALID_SOCKET){
		_sockfd = sock;
		memset(_msg_Recv_Buff, 0, sizeof(_msg_Recv_Buff));
		_last_recv_pos = 0;

		memset(_msg_Send_Buff, 0, sizeof(_msg_Send_Buff));
		_last_send_pos = 0;

		_heartTime = 0;
		_timing = 0;
	}

	SOCKET Sockfd(){
		return _sockfd;
	}

	char* Msg_recv_Buff(){
		return _msg_Recv_Buff;
	}

	int getLastPos(){
		return  _last_recv_pos;
	}

	void setLastPos(int last_pos){
		_last_recv_pos = last_pos;
	}

	int SendAllData(DataHeader* dh) {
		SendData(dh);
		SendAllData();
	}

	int SendAllData() {
		int ret = SOCKET_ERROR;
		if (_last_send_pos > 0 && INVALID_SOCKET != _sockfd) {
			ret = send(_sockfd, _msg_Send_Buff, _last_send_pos, 0);
			_last_send_pos = 0;
			resetTiming();
			if (ret == SOCKET_ERROR) {
				return ret;
			}
		}
		
		return ret;
	}

	int SendData(DataHeader* dh){
		int ret = SOCKET_ERROR;
		int nlen = dh->dataLength;
		const char* pSendData = (const char*)dh;
		while (true){
			if (_last_send_pos + nlen >= SEND_BUFF_SIZE){
				int copyLength = SEND_BUFF_SIZE - _last_send_pos;
				memcpy(_msg_Send_Buff + _last_send_pos, pSendData, copyLength);
				pSendData += copyLength;
				nlen -= copyLength;
				ret = send(_sockfd, _msg_Send_Buff, SEND_BUFF_SIZE, 0);
				_last_send_pos = 0;
				resetTiming();
				if (ret == SOCKET_ERROR){
					return ret;
				}
			}
			else{
				memcpy(_msg_Send_Buff + _last_send_pos, pSendData, nlen);
				_last_send_pos += nlen;
				break;
			}
		}
		return ret;
	}

	void resetHeartTime() {
		_heartTime = 0;
	}

	bool checkHeart(time_t dateTime) {
		_heartTime += dateTime;
		if (_heartTime >= MAX_HEARTTIMEOUT) {
			printf("client=%d,_heartTime=%d\n", _sockfd, _heartTime);
			return true;
		}

		return false;
	}

	void resetTiming() {
		_timing = 0;
	}

	bool checkTiming(time_t dateTime) {
		_timing += dateTime;
		if (_timing >= MAX_TIMINGOUT) {
			//printf("client=%d,_timing=%d\n", _sockfd, _timing);
			//重置_timing
			SendAllData();
			resetTiming();
			return true;
		}

		return false;
	}
};

#endif // !_CELLCLIENT_hpp_
