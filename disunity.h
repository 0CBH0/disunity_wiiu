#pragma once

#define CHUNK 16384
#define LEVEL 6

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef signed   char	s8;
typedef signed   short	s16;
typedef signed   int    s32;
typedef unsigned long	ulg;
typedef unsigned long long u64;
typedef signed long long   s64;

u8  tempa_u8,
    tempb_u8;

u16 tempa_u16,
    tempb_u16;

u32 tempa_u32,
    tempb_u32;

ulg pos;

typedef struct {
	u32 m_PathID;
	u32 Offset;
	u32 NewOffset;
	u32 Size;
	u32 NewSize;
	u32 Type1;
	u32 Type2;
	char name[256];
} AssetPreloadData;
