#include "chunkchunk.h"

#define DECLMYSELF  MyChunkChunkSelf * const pSelf = (void*)pCC
typedef struct MyChunkChunkSelf MyChunkChunkSelf;
struct MyChunkChunkSelf
{
	TCHAR szChunk[CHUNKCHUNK_SZCHUNK_SIZE];
	TCHAR const *pszFullStr;
	size_t offset;
};

EXTERN_C void ChunkChunkInit(
	ChunkChunk *pCC,
	TCHAR const *pszFullStr
) {
	DECLMYSELF;
	memset(pCC, 0, sizeof(*pCC));
	pSelf->pszFullStr = pszFullStr;
}

EXTERN_C BOOL ChunkChunkNext(
	ChunkChunk *pCC
) {
	DECLMYSELF;
	size_t offset2 = pSelf->offset;
	size_t cchChunk = 0;
	BOOL is_in_dquote = FALSE;
	memset(pSelf->szChunk, 0, sizeof(pSelf->szChunk));
	if (pSelf->pszFullStr[offset2] == ' ')
	{
		while (pSelf->pszFullStr[offset2] == ' ')
		{
			++(offset2);
		}
	}
	for (;;)
	{
		TCHAR const c = pSelf->pszFullStr[offset2 + cchChunk];
		if (c == 0) break;
		if (!is_in_dquote && c == ' ') break;
		if (c == '"') is_in_dquote = !is_in_dquote;
		++(cchChunk);
	}
	pSelf->offset = offset2 + cchChunk;
	if (cchChunk > CHUNKCHUNK_SZCHUNK_SIZE)
	{
		cchChunk = CHUNKCHUNK_SZCHUNK_SIZE;
	}
	(void)lstrcpyn(pSelf->szChunk, &pSelf->pszFullStr[offset2],
		(int)min(cchChunk + 1, CHUNKCHUNK_SZCHUNK_SIZE));
	return pSelf->szChunk[0] != 0;
}
