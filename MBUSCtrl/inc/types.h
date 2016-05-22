#pragma once

#ifndef _TYPES_h
#define _TYPES_h


/**************************************************************************
* definitions
**************************************************************************/

//
// easy to type types, type
//
typedef unsigned char  u08;
typedef unsigned short u16;
typedef unsigned long  u32;

typedef char  s08;
typedef short s16;
typedef long  s32;

typedef u16 hsize_t;

struct u16bytes
{
	u08 low, high;
};

union u16convert
{
	u16 value;
	struct u16bytes bytes;
};
struct u32bytes
{
	u08 byte1, byte2, byte3, byte4;
};

struct u32words
{
	u16 low, high;
};

union u32convert
{
	u32 value;
	struct u32bytes bytes;
	struct u32words words;
};

#define WORD    u16
#define BYTE    u08
#define DWORD   u32

//#define NULL    0

#define ON  1
#define OFF 0

#define TRUE  1
#define FALSE 0

#define true  1
#define false 0

typedef void * HANDLE;


/**************************************************************************
* declarations
**************************************************************************/


/**************************************************************************
* function prototypes
**************************************************************************/



#endif /* _TYPES_h */