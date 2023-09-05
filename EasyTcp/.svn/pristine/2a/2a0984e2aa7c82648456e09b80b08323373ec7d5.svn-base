#include "Alloc.h"
#include "MemoryMgr.hpp"

void* operator new(size_t nsize) {
	return MemoryMgr::Instance().allocMem(nsize);
}

void* operator new[](size_t nsize) {
	return MemoryMgr::Instance().allocMem(nsize);
}

void operator delete(void* p) {
	MemoryMgr::Instance().freeMem(p);
}

void operator delete[](void* p) {
	MemoryMgr::Instance().freeMem(p);
}

void* alloc_mem(size_t nsize) {
	return malloc(nsize);
}

void free_mem(void* p) {
	free(p);
}