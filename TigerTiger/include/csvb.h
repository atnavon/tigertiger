#pragma once

#include "common.h"

#define CSVB_MAGIC 'BVSC'

typedef struct csvb_header_t
{
	uint32 magic;
	uint16 row_num;
	uint16 cell_num;
	uint32 val_size;
	uint32 str_size;
} csvb_header_t;

enum csvb_type
{
	CSVB_TYPE_INT,
	CSVB_TYPE_FLOAT,
	CSVB_TYPE_NULL,
	CSVB_TYPE_STR,
};

typedef struct csvb_row_t
{
	uint16 col_num;
	uint16 cell_idx;
} csvb_row_t;

typedef struct csvb_cell_t
{
	uint8 type;
	union
	{
		int32 ival;
		float fval;
		const char* sval;
	};
} csvb_cell_t;

typedef struct csvb_t
{
	uint16 row_num;
	uint16 cell_num;
	csvb_row_t* rows;
	csvb_cell_t* cells;
	void* str_data;
} csvb_t;

uint8 csvb_init(csvb_t* csvb, const void* data, uint64 size);
void csvb_fini(csvb_t* csvb);
uint32 csvb_row_count(csvb_t* csvb);
uint32 csvb_column_count(csvb_t* csvb, uint32 row_idx);
uint8 csvb_get_type(csvb_t* csvb, uint32 row_idx, uint32 col_idx);
int32 csvb_read_int(csvb_t* csvb, uint32 row_idx, uint32 col_idx);
float csvb_read_float(csvb_t* csvb, uint32 row_idx, uint32 col_idx);
const char* csvb_read_string(csvb_t* csvb, uint32 row_idx, uint32 col_idx);
