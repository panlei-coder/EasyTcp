//#include"Alloctor.cpp"
#include"CELLObjectPool.hpp"
#include "CELLTimestamp.hpp"
#include<stdlib.h>
#include<thread>
#include<iostream>
#include<stdio.h>

class ClassA:public ObjectPoolBase<ClassA,100000>{
public:
	ClassA(int n) {
		num = n;
		printf("ClassA\n");
	}

	~ClassA() {
		printf("~ClassA\n");
	}

private:
	int num = 0;
	int sum[2000];
};
//int a = sizeof(ClassA);
class ClassB:public ObjectPoolBase<ClassB,100000>{
public:
	ClassB(int n,int m) {
		num = n * m;
		printf("ClassB\n");
	}
	
	~ClassB() {
		printf("~ClassB\n");
	}

private:
	int num = 0;
};

const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount / tCount;
void workFun() {
	ClassA* data[nCount];
	for (int i = 0; i < nCount; i++) {
		data[i] = ClassA::createObject(nCount);
	}

	for (int i = 0; i < nCount; i++) {
		ClassA::destroyObject(data[i]);
	}
}

int main() {
	/*char* data1 = new char[128];
	delete[] data1;

	char* data2 = new char[64];
	delete[] data2;

	char* data3 = new char;
	delete(data3);*/
	/*char* data[1100];
	for (size_t n = 0; n < 1100; n++) {
		data[n] = new char[n+1];
	}
	for (size_t n = 0; n < 1100; n++) {
		delete[] data[n];
	}*/

	/*std::thread* memory_Thread[tCount];
	for (int i = 0; i < tCount; i++) {
		memory_Thread[i] = new std::thread(workFun);
	}
	CELLTimestamp time;
	for (int i = 0; i < tCount; i++) {
		memory_Thread[i]->join();
	}
	
	printf("%lf", time.getElapsedTimeInMilliSec());

	for (int i = 0; i < tCount; i++) {
		delete memory_Thread[i];
	}*/

	//std::thread t[tCount];
	//for (int n = 0; n < tCount; n++)
	//{
	//	t[n] = std::thread(workFun);
	//}
	//CELLTimestamp tTime;
	//for (int n = 0; n < tCount; n++)
	//{
	//	t[n].join();
	//	//t[n].detach();
	//}
	//std::cout << tTime.getElapsedTimeInMilliSec() << std::endl;
	//std::cout << "Hello,main thread." << std::endl;

	printf("----0----\n");
	{
		ClassA* a1 = new ClassA(1);
		std::shared_ptr<ClassA> a0(a1);
		//std::shared_ptr<ClassA> a2 = std::make_shared<ClassA>(2);
		//std::shared_ptr<ClassA> a3(new ClassA(3));
	}
	printf("----1----\n");
	{
		std::shared_ptr<ClassA> a4 = std::make_shared<ClassA>(4);
	}
	printf("----2----\n");
	{
		std::shared_ptr<ClassA> a5(new ClassA(5));
	}
	printf("----3----\n");

	ClassA* a6 = new ClassA(6);
	delete a6;
	printf("----4----\n");

	return 0;
}