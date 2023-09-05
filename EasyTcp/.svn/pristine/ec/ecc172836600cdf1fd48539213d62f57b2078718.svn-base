#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_

#include<mutex>
#include<stdlib.h>
#include<assert.h>
#include "Alloc.h"

#ifdef _DEBUG
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)
	#endif // !xPrintf
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif // !xPrintf
#endif // _DEBUG

template<class Type,size_t nPoolSize>
class CELLObjectPool {
private:
	//单个块头部信息
	class NodeHeader {
	public:
		//下一块位置
		NodeHeader* pNext;
		//内存块的编号
		int nID;
		//内存块的使用次数
		char nRef;
		//是否在内存池中
		bool bPool;
	private:
		//预留空间
		char c1;
		char c2;
	};
	//对象池头部内存单元
	NodeHeader* _pHeader;
	//对象池的地址
	char* _pBuf;
	//共享资源，需要加锁
	std::mutex _mutex;
public:
	CELLObjectPool() {
		initPool();
	}
	
	~CELLObjectPool() {
		if (_pBuf) {
			delete[] _pBuf;
		}
	}

private:
	//初始化对象池
	void initPool() {
		assert(_pBuf == nullptr);
		if (_pBuf) {
			return;
		}

		size_t realSize = sizeof(NodeHeader) + sizeof(Type);
		size_t n = nPoolSize * realSize;
		//申请对象池的内存
		_pBuf = new char[n];
		//初始化内存池
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;

		//遍历内存块进行初始化
		NodeHeader* pTemp1 = _pHeader;
		for (size_t n = 1; n < nPoolSize; n++) {
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + n*realSize);
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
public:
	//申请对象内存
	void* allocObjMemory(size_t nSize) {
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;
		if (nullptr == _pHeader) {
			NodeHeader* pReturn = (NodeHeader*)new char[sizeof(Type) + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = pReturn->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocObjMemory:%llx,id=%d,size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(NodeHeader));
	}
	//释放对象内存
	void freeObjMemory(void* pMemory) {
		NodeHeader* pBlock = (NodeHeader*)((char*)pMemory - sizeof(NodeHeader));
		xPrintf("freeObjMemory:%llx,id=%d\n", pBlock, pBlock->nID);
		assert(pBlock->nRef == 1);
		pBlock->nRef--;
		if (pBlock->bPool) {
			std::lock_guard < std::mutex > lg(_mutex);
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			delete[] pBlock;
		}
	}
};

template<class Type, size_t nPoolSize>
class ObjectPoolBase {
public:
	void* operator new(size_t nSize) {
		return objectPool().allocObjMemory(nSize);
	}

	void operator delete(void* pMemory) {
		objectPool().freeObjMemory(pMemory);
	}

	template<typename ...Args>
	static Type* createObject(Args ...args) {
		Type* obj = new Type(args...);
		return obj;
	}

	static void destroyObject(Type* obj) {
		delete obj;
	}
private:
	typedef CELLObjectPool<Type, nPoolSize> ClassTypePool;
	static ClassTypePool& objectPool() {
		static ClassTypePool sPool;
		return sPool;
	}
};

#endif // !_CELLObjectPool_hpp_