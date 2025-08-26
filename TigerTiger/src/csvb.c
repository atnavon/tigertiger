#include "csvb.h"

uint8 csvb_init(csvb_t* csvb, const void* data, uint64 size)
{
	memset(csvb, 0, sizeof(csvb_t));

	const csvb_header_t* header = data;

	if (!header || header->magic != CSVB_MAGIC)
	{
		return 0;
	}

	const uint16* col_counts = MEMOFFS(header, sizeof(csvb_header_t));
	const int16* cell_types = MEMOFFS(col_counts, header->row_num * sizeof(uint16));
	const void* value_data = MEMOFFS(cell_types, header->cell_num * sizeof(int16));
	const void* string_data = MEMOFFS(value_data, header->val_size);
	uint64 total_size = ((uint64)string_data + header->str_size - (uint64)data);

	if (total_size > size)
	{
		return 0;
	}

	if (header->row_num > 0)
	{
		csvb->row_num = header->row_num;
		csvb->rows = malloc(csvb->row_num * sizeof(csvb_row_t));

		uint16 cursor = 0;

		for (uint32 i = 0; i < csvb->row_num; ++i)
		{
			csvb_row_t* row = &csvb->rows[i];

			row->col_num = col_counts[i];
			row->cell_idx = cursor;

			cursor += col_counts[i];
		}
	}

	if (header->cell_num > 0)
	{
		if (header->str_size > 0)
		{
			csvb->str_data = malloc(header->str_size);

			memcpy(csvb->str_data, string_data, header->str_size);
		}

		csvb->cell_num = header->cell_num;
		csvb->cells = malloc(csvb->cell_num * sizeof(csvb_cell_t));

		uint32 cursor = 0;

		for (uint32 i = 0; i < csvb->cell_num; ++i)
		{
			csvb_cell_t* cell = &csvb->cells[i];

			switch (cell_types[i])
			{
				case -5:
					cell->type = CSVB_TYPE_NULL;
					break;
				case -4:
					cell->type = CSVB_TYPE_INT;
					cell->ival = *(const int8*)MEMOFFS(value_data, cursor);
					cursor += sizeof(int8);
					break;
				case -3:
					cell->type = CSVB_TYPE_INT;
					cell->ival = *(const int16*)MEMOFFS(value_data, cursor);
					cursor += sizeof(int16);
					break;
				case -2:
					cell->type = CSVB_TYPE_INT;
					cell->ival = *(const int32*)MEMOFFS(value_data, cursor);
					cursor += sizeof(int32);
					break;
				case -1:
					cell->type = CSVB_TYPE_FLOAT;
					cell->fval = *(const float*)MEMOFFS(value_data, cursor);
					cursor += sizeof(float);
					break;
				default:
					cell->type = CSVB_TYPE_STR;
					cell->sval = MEMOFFS(csvb->str_data, cell_types[i]);
					break;
			}
		}
	}

	return 1;
}

void csvb_fini(csvb_t* csvb)
{
	free(csvb->rows);
	free(csvb->cells);
	free(csvb->str_data);

	csvb->row_num = 0;
	csvb->cell_num = 0;

	memset(csvb, 0, sizeof(csvb_t));
}

uint32 csvb_row_count(csvb_t* csvb)
{
	return csvb->row_num;
}

uint32 csvb_column_count(csvb_t* csvb, uint32 row_idx)
{
	const csvb_row_t* row = &csvb->rows[row_idx];

	return row->col_num;
}

uint8 csvb_get_type(csvb_t* csvb, uint32 row_idx, uint32 col_idx)
{
	const csvb_row_t* row = &csvb->rows[row_idx];
	const csvb_cell_t* cell = &csvb->cells[row->cell_idx + col_idx];

	return cell->type;
}

int32 csvb_read_int(csvb_t* csvb, uint32 row_idx, uint32 col_idx)
{
	const csvb_row_t* row = &csvb->rows[row_idx];
	const csvb_cell_t* cell = &csvb->cells[row->cell_idx + col_idx];

	if (cell->type == CSVB_TYPE_INT)
	{
		return cell->ival;
	}
	else if (cell->type == CSVB_TYPE_FLOAT)
	{
		return (int32)cell->fval;
	}

	return 0;
}

float csvb_read_float(csvb_t* csvb, uint32 row_idx, uint32 col_idx)
{
	const csvb_row_t* row = &csvb->rows[row_idx];
	const csvb_cell_t* cell = &csvb->cells[row->cell_idx + col_idx];

	if (cell->type == CSVB_TYPE_FLOAT)
	{
		return cell->fval;
	}
	else if (cell->type == CSVB_TYPE_INT)
	{
		return (float)cell->ival;
	}

	return 0.0f;
}

const char* csvb_read_string(csvb_t* csvb, uint32 row_idx, uint32 col_idx)
{
	const csvb_row_t* row = &csvb->rows[row_idx];
	const csvb_cell_t* cell = &csvb->cells[row->cell_idx + col_idx];

	if (cell->type == CSVB_TYPE_STR)
	{
		return cell->sval;
	}

	return NULL;
}
