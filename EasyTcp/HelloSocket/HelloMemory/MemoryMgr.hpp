#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_

#include<stdlib.h>
#include<assert.h>
#include<mutex>
#include "Alloc.h"

#ifdef _DEBUG  //�����debug״̬
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)
	#endif // !xPrintf
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif // !xPrintf
#endif

#define MAX_MEMORY_SIZE 1024
class MemoryAlloc;

//�ڴ�� ��С��Ԫ
class MemoryBlock {
public:
	//�����ڴ��
	MemoryAlloc* pAlloc;   //�����ڴ����ʱ���ö�Ӧ�������ڴ�ؽ����ڴ��ͷ�
	//��һ���ڴ�� ��С��Ԫ��λ��
	MemoryBlock* pNext;
	//�ڴ����
	int  nID;
	//���ô���
	int  nRef;
	//�Ƿ����ڴ����
	bool bPool;

private:
	//Ԥ��
	char c1;
	char c2;
	char c3;
};
//32λ������Ϊ20B��64λ������Ϊ32B
//const int MemoryBlockSize = sizeof(MemoryBlock);

//�ڴ��
class MemoryAlloc{
protected:
	//�ڴ�ص�ַ
	char* _pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//������С�ڴ��Ĵ�С
	size_t _nSize;
	//�����ڴ����������С�ڴ��ĸ���
	size_t _nBlockCount;
	//�������ڴ���ͷ��ڴ���е��ڴ�����μ���
	std::mutex _mutex;
public:
	MemoryAlloc() {
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockCount = 0;
 	}
	~MemoryAlloc() {
		if (_pBuf) {
			free(_pBuf);
		}
	}
	
	void initMemory() {
		xPrintf("initMemory\n");
		//����
		assert(nullptr == _pBuf); //ֻ����debug��ģʽ��������,Ϊ�ٵ�ʱ��Ż����
		if (_pBuf) {
			return;
		}

		//�����ڴ�صĴ�С
		size_t bufSize = (_nSize+ sizeof(MemoryBlock))* _nBlockCount;
		//��ϵͳ����ص��ڴ�ռ�
		_pBuf = (char*)malloc(bufSize);

		//��ʼ���ڴ��
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		
		//�����ڴ����г�ʼ��
		MemoryBlock* pTemp1 = _pHeader;
		for (size_t n = 1; n < _nBlockCount; n++) {
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (_nSize + sizeof(MemoryBlock)) * n);
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

	//�����ڴ�
	void* allocMemory(size_t nSize) {
		std::lock_guard<std::mutex> lg(_mutex);

		if (!_pBuf) {
			initMemory();
		}

		MemoryBlock* pReturn = nullptr;
		if (nullptr == _pHeader) {
			pReturn = (MemoryBlock*)malloc(_nSize + sizeof(MemoryBlock));
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
		}
		else {
			pReturn = _pHeader;
			_pHeader = pReturn->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		xPrintf("Alloc:<addr=%llx>,<nID=%d>,<nRef=%d>\n", pReturn, pReturn->nID, pReturn->nRef);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	//�ͷ��ڴ�
	void freeMemory(void* pMemory) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMemory - sizeof(MemoryBlock));
		assert(pBlock->nRef == 1);
		
		if (--pBlock->nRef != 0) {
			return;  //�е�û��Ҫ
		}
		xPrintf("Free:<addr=%llx>,<nID=%d>,<nRef=%d>\n", pBlock, pBlock->nID, pBlock->nRef);
		//��Ҫ�ж��ͷŵ����ڴ���λ���Ƿ����ڴ����
		if (pBlock->bPool) {//���ڴ��ֻ��Ҫ���ڴ�������ͷָ��������
			std::lock_guard<std::mutex> lg(_mutex);
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {//�����ڴ����ֱ���ͷŵ�����
			free(pBlock);
		}
	}
};

template<size_t nSize,size_t nBlockCount>
class MemoryAlloctor:public MemoryAlloc{
public:
	MemoryAlloctor() {
		const size_t n = sizeof(void*); //ָ��Ĵ�С��������4���뻹��8����
		_nSize = (nSize / n) * n + (nSize % n ? n : 0);
		_nBlockCount = nBlockCount;
	}
};

//�ڴ������
class MemoryMgr {
private:
	MemoryAlloctor<64, 1000> _mem64;  //�����޲����Ĺ������
	MemoryAlloctor<128, 1000> _mem128;
	MemoryAlloctor<256, 1000> _mem256;
	MemoryAlloctor<512, 1000> _mem512;
	MemoryAlloctor<1024, 1000> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];//�ڴ�ص�ӳ�����飬����ֱ�ӷ���ָ���ڴ��
private:
	MemoryMgr() {
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}

	~MemoryMgr() {

	}
public:
	static MemoryMgr& Instance() {
		//����ģʽ ��̬  ����MemoryMgr�Ķ�������
		static MemoryMgr mgr; //�����޲ι��캯��
		return mgr;    //���ض�������
	}
private:
	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemAlloc) {
		xPrintf("init_szAlloc:<%d,%d>\n",nBegin,nEnd);
		for (int n = nBegin; n <= nEnd; n++) {
			_szAlloc[n] = pMemAlloc;
		}
	}
public:
	//�����ڴ������ü���
	void addRef(void* pMemory) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMemory - sizeof(MemoryBlock));
		pBlock->nRef++;
	}
	//�����ڴ�
	void* allocMem(size_t nSize) {
		if (nSize <= MAX_MEMORY_SIZE) {
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else {
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			xPrintf("Alloc:<addr=%llx>,<nID=%d>,<nRef=%d>\n", pReturn, pReturn->nID, pReturn->nRef);
			return ((char*)pReturn+sizeof(MemoryBlock));
		}
	}
	//�ͷ��ڴ�
	void  freeMem(void* pMemory) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMemory - sizeof(MemoryBlock));
		if (pBlock->bPool) {
			pBlock->pAlloc->freeMemory(pMemory);
		}
		else {
			xPrintf("Free:<addr=%llx>,<nID=%d>,<nRef=%d>\n", pBlock, pBlock->nID, pBlock->nRef);
			if (pBlock->nRef-- == 0) {
				free(pBlock);
			}
		}
	}
};
#endif //_MemoryMgr_hpp_