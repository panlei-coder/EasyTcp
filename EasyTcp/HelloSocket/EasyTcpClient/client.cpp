#include "EasyTcpClient.hpp"
#include<thread>

bool g_Run = true;
void cmdThread(){
	while(true){ 
		char order[128] = {};
		scanf("%s", order); //scanf_s("%s",order,128)

		if (0 == strcmp(order, "exit")){
			g_Run = false;
			printf("�˳�cmd_thread�߳�...\n");
			break;
		}
		else	{
			printf("��֧�ֵ�����...\n");
		}
	}
}

//�ͻ�������
const int cCount = 4;
//�����߳�����
const int tCount = 1;
//�ͻ�������
EasyTcpClient* client[cCount];
bool connected[tCount];

void recvThread(int begin,int end) {
	while (g_Run) {
		for (int n = begin; n < end; n++) {
			client[n]->OnRun();
		}
	}
}

void sendThread(int id){
	//4���߳� ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;

	for (int n = begin; n < end; n++){
		client[n] = new EasyTcpClient;
	}
	for (int n = begin; n < end; n++){
		client[n]->Connect("127.0.0.1", 4567);
	}
	connected[id-1] = true;

	int t;
	while (true){
		t = 0;
		for (int i = 0; i < tCount; i++){
			if (connected[i]){
				t++;
			}
		}

		if (t == tCount){
			break;
		}
	}

	std::thread recv_thread(recvThread, begin, end);
	recv_thread.detach();

	Login login[10];
	for (int i = 0; i < 10; i++) {
		strcpy(login[i].password, "pl");
		strcpy(login[i].username, "pl");
	}
	
	while (g_Run){
		for (int n = begin; n < end; n++){
			client[n]->SendData(login,sizeof(login));
		}
	}

	for (int n = begin; n < end; n++){
		client[n]->Close();
		delete client[n];
	}
}

int main(){
	std::thread th(cmdThread);
	th.detach();  //�����߳������̷߳��룬�߳̽���ʱ�����������Դ��ֱ�ӱ��ͷŵ�

	std::thread* t[tCount];
	for (int n = 0; n < tCount; n++){
		connected[n] = false;
		t[n] = new std::thread(sendThread, n + 1);
		t[n]->detach();
	}

	printf("���˳�...\n");
	getchar();
	return 0;
}
