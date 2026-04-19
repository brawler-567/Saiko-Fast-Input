 
 

#ifndef BASE_MEM_H
#define BASE_MEM_H

#include <cstdint>
#include <cstring>
#include <type_traits>

 

 
void mem_copy(void *dest, const void *source, size_t size);

 
void mem_move(void *dest, const void *source, size_t size);

 
template<typename T>
inline void mem_zero(T *block, size_t size)
{
	static_assert((std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value) || std::is_fundamental<T>::value);
	memset(block, 0, size);
}

 
int mem_comp(const void *a, const void *b, size_t size);

 
bool mem_has_null(const void *block, size_t size);

#endif
