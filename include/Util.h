#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <time.h>

struct Random
{
	template<typename T>
	static inline T GetNumber(T min, T max)
	{
		return T(min + rand() / (float)RAND_MAX * (max - min));
	}
};

struct Pointer
{
	static inline void Delete(void *p)
	{
		if (!p) return;
		delete p;
		p = NULL;
	}
};

struct Array
{
	static inline void Delete(void **p, GLuint count)
	{
		if (!p) return;
		while (count) Pointer::Delete(p[--count]);
		delete[] p;
		p = NULL;
	}
};

#endif