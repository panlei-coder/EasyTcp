#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_

#include<stdlib.h>
#include<assert.h>
#include<mutex>
#include "Alloc.h"

#ifdef _DEBUG  //如果事debug状态
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

//内存块 最小单元
class MemoryBlock {
public:
	//所属内存池
	MemoryAlloc* pAlloc;   //便于内存管理时调用对应的所属内存池进行内存释放
	//下一块内存块 最小单元的位置
	MemoryBlock* pNext;
	//内存块编号
	int  nID;
	//引用次数
	int  nRef;
	//是否在内存池中
	bool bPool;

private:
	//预留
	char c1;
	char c2;
	char c3;
};
//32位环境下为20B，64位环境下为32B
//const int MemoryBlockSize = sizeof(MemoryBlock);

//内存池
class MemoryAlloc{
protected:
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//单个最小内存块的大小
	size_t _nSize;
	//单个内存池中所含最小内存块的个数
	size_t _nBlockCount;
	//对申请内存和释放内存池中的内存块代码段加锁
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
		//断言
		assert(nullptr == _pBuf); //只有在debug的模式下其作用,为假的时候才会调用
		if (_pBuf) {
			return;
		}

		//计算内存池的大小
		size_t bufSize = (_nSize+ sizeof(MemoryBlock))* _nBlockCount;
		//向系统申请池的内存空间
		_pBuf = (char*)malloc(bufSize);

		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		
		//遍历内存块进行初始化
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

	//申请内存
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

	//释放内存
	void freeMemory(void* pMemory) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMemory - sizeof(MemoryBlock));
		assert(pBlock->nRef == 1);
		
		if (--pBlock->nRef != 0) {
			return;  //有点没必要
		}
		xPrintf("Free:<addr=%llx>,<nID=%d>,<nRef=%d>\n", pBlock, pBlock->nID, pBlock->nRef);
		//需要判断释放掉的内存块的位置是否在内存池中
		if (pBlock->bPool) {//在内存池只需要将内存池链表的头指向它即可
			std::lock_guard<std::mutex> lg(_mutex);
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {//不在内存池中直接释放掉就行
			free(pBlock);
		}
	}
};

template<size_t nSize,size_t nBlockCount>
class MemoryAlloctor:public MemoryAlloc{
public:
	MemoryAlloctor() {
		const size_t n = sizeof(void*); //指针的大小决定了是4对齐还是8对齐
		_nSize = (nSize / n) * n + (nSize % n ? n : 0);
		_nBlockCount = nBlockCount;
	}
};

//内存管理工具
class MemoryMgr {
private:
	MemoryAlloctor<64, 1000> _mem64;  //调用无参数的构造参数
	MemoryAlloctor<128, 1000> _mem128;
	MemoryAlloctor<256, 1000> _mem256;
	MemoryAlloctor<512, 1000> _mem512;
	MemoryAlloctor<1024, 1000> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];//内存池的映射数组，可以直接访问指定内存块
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
		//单例模式 静态  返回MemoryMgr的对象引用
		static MemoryMgr mgr; //调用无参构造函数
		return mgr;    //返回对象引用
	}
private:
	//初始化内存池映射数组
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemAlloc) {
		xPrintf("init_szAlloc:<%d,%d>\n",nBegin,nEnd);
		for (int n = nBegin; n <= nEnd; n++) {
			_szAlloc[n] = pMemAlloc;
		}
	}
public:
	//增加内存块的引用计数
	void addRef(void* pMemory) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMemory - sizeof(MemoryBlock));
		pBlock->nRef++;
	}
	//申请内存
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
	//释放内存
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