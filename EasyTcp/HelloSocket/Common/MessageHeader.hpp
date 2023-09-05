#ifndef _MessageHeader_hpp_   //Ԥ���������ֹ�ظ�����
#define _MessageHeader_hpp_

enum cmd{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

//���ݰ�����ͷ
struct DataHeader{
	DataHeader(){
		dataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLength;
	short cmd;
};

struct Login :public DataHeader{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	};
	char username[32];
	char password[32];
	char data[32];
};

struct LoginResult :public DataHeader{
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	};
	int result;
	char data[92];
};

struct LoginOut : public DataHeader{
	LoginOut() {
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	};
	char username[32];
};

struct LoginOutResult : public DataHeader{
	LoginOutResult() {
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	};
	int result;
};

struct NewUserJoin : public DataHeader{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	};
	int sock;   //�¼���Ŀͻ���socket
};
#endif // !_MessageHeader_hpp_