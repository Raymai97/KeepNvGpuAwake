#include <stddef.h>
#include <string.h>

#ifndef EXTERN_C
# if defined(__cplusplus)
#  define EXTERN_C extern "C"
# else
#  define EXTERN_C
# endif
#endif

/* ------ PATCH: Avoid msvcrt on msvc2012 and up ------- */

#if defined(_MSC_VER) && (_MSC_VER) >= 1700

#pragma intrinsic(memset)

EXTERN_C void * __cdecl memset(void *, int, size_t);

#pragma function(memset)

EXTERN_C void * __cdecl memset(void *pBuf, int val, size_t cb)
{
	unsigned char *p = pBuf;
	while (cb-- > 0) {
		*p++ = (unsigned char)(val);
	}
	return pBuf;
}

#pragma intrinsic(memcpy)

EXTERN_C void * __cdecl memcpy(void *, void const *, size_t);

#pragma function(memcpy)

EXTERN_C void * __cdecl memcpy(void *pDst, void const *pSrc, size_t cbSrc)
{
	typedef unsigned char byte_t;
	byte_t *p = pDst;
	byte_t const *q = pSrc;
	while (cbSrc-- > 0) {
		*p++ = *q++;
	}
	return pDst;
}

#pragma intrinsic(memcmp)

EXTERN_C int __cdecl memcmp(void const *p1, void const *p2, size_t cb);

#pragma function(memcmp)

EXTERN_C int __cdecl memcmp(void const *p1, void const *p2, size_t cb)
{
	typedef unsigned char byte_t;
	byte_t const *p = p1, *q = p2;
	while (cb-- > 0) {
		if (p[cb] < q[cb]) { return -1; }
		if (p[cb] > q[cb]) { return 1; }
	}
	return 0;
}

#endif/* msvc2012++ and defined(NDEBUG) */
