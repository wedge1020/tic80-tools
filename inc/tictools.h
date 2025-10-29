#ifndef _TICTOOLS_H
#define _TICTOOLS_H

#define  CHUNK_TILES         1
#define  CHUNK_SPRITES       2
#define  CHUNK_COVER_DEP     3 // deprecated as of 0.90
#define  CHUNK_MAP           4
#define  CHUNK_CODE          5
#define  CHUNK_FLAGS         6
#define  CHUNK_SAMPLES       9
#define  CHUNK_WAVEFORM     10
#define  CHUNK_PALETTE      12
#define  CHUNK_PATTERNS_DEP 13 // deprecated as of 0.80
#define  CHUNK_MUSIC        14
#define  CHUNK_PATTERNS     15
#define  CHUNK_CODE_ZIP     16 // deprecated as of 1.00
#define  CHUNK_DEFAULT      17
#define  CHUNK_SCREEN       18
#define  CHUNK_BINARY       19
//7, 8, 11    (reserved) ...        
//19..31    (reserved) ...

#define  FLAG_DEFAULT        0
#define  FLAG_VERBOSE        1
#define  FLAG_PROMPT         2

typedef unsigned char      ussi;
typedef unsigned short int usi;
typedef unsigned int       ui;

struct chunk {
	ui            offset;
	ussi          bank;
	ussi          type;
	usi           size;
	ussi          reserved;
	ussi         *data;
	struct chunk *next;
};
typedef struct chunk Chunk;

struct list {
	Chunk        *start;
	struct list  *next;
};
typedef struct list List;

Chunk  *inventory (char *);

#endif
