#pragma once

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

typedef struct
{
	uint8 r;
	uint8 g;
	uint8 b;
} rgb_t;

typedef struct
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
} rgba_t;

#define ARRAYNUM(x) (sizeof(x) / sizeof(x[0]))
#define MEMOFFS(mem, offs) (void*)((uint64)(mem) + (offs))
#define ALIGNUP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(x, min, max) MIN(MAX(x, min), max)
#define MAD(x, y, z) (((x) * (y)) + (z))
#define ISIN(x, min, max) ((x) >= (min) && (x) <= (max))
#define SIGN(x) ((x) < 0 ? -1 : 1)
#define SIGNZ(x) ((x) < 0 ? -1 : (x) > 0 ? 1 : 0)
#define PI 3.14159f
