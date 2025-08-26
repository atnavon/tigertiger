#pragma once

#include "common.h"

#define SAR1_MAGIC 'SAR1'

typedef struct sar1_header_t
{
	uint32 magic;
	uint32 size;
	uint32 version;
	uint32 file_num;
	uint32 file_offs;
	uint32 data_offs;
	uint32 _pad1;
	uint32 _pad2;
	char path[128];
} sar1_header_t;

typedef struct sar1_file_t
{
	uint32 offs;
	uint32 size;
	uint32 hash;
	char name[32];
} sar1_file_t;

typedef struct sar1_t
{
	const sar1_header_t* header;
	uint32 stride;
} sar1_t;

uint8 sar1_init(sar1_t* sar1, const void* data);
void sar1_fini(sar1_t* sar1);
const void* sar1_file_addr(sar1_t* sar1, uint32 file_idx);
const void* sar1_file_meta(sar1_t* sar1, uint32 file_idx);
uint32 sar1_file_count(sar1_t* sar1);
