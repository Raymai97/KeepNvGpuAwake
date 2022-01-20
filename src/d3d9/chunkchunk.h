#pragma once
#include <Windows.h>

// "szChunk" is always zero-terminated.
// If chunk is bigger than buffer, it will be truncated.
#define CHUNKCHUNK_SZCHUNK_SIZE  (256)

typedef struct ChunkChunk ChunkChunk;
struct ChunkChunk
{
	TCHAR szChunk[CHUNKCHUNK_SZCHUNK_SIZE];
	LPARAM internal_data[4];
};

EXTERN_C void ChunkChunkInit(
	ChunkChunk *pCC,
	TCHAR const *pszFullStr
);

EXTERN_C BOOL ChunkChunkNext(
	ChunkChunk *pCC
);
