#ifndef _Alloc_h_
#define _Alloc_h_

void* operator new(size_t nsize);
void* operator new[](size_t nsize);
void operator delete(void* p);
void operator delete[](void* p);
void* malloc_mem(size_t nsize);
void  free_mem(void* p);
#endif //_Alloc_h_
