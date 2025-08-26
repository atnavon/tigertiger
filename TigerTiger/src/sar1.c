#include "sar1.h"


uint8 sar1_init(sar1_t* sar1, const void* data)
{
	memset(sar1, 0, sizeof(sar1_t));

	const sar1_header_t* header = data;

	if (!header || header->magic != SAR1_MAGIC)
	{
		return 0;
	}

	sar1->header = header;
	sar1->stride = sizeof(sar1_file_t) + 20;

	if (sar1->header->version >= 0x200)
	{
		sar1->stride += 32;
	}

	return 1;
}

void sar1_fini(sar1_t* sar1)
{
	memset(sar1, 0, sizeof(sar1_t));
}

const void* sar1_file_addr(sar1_t* sar1, uint32 file_idx)
{
	if (file_idx >= sar1->header->file_num)
	{
		return NULL;
	}

	const sar1_file_t* file = MEMOFFS(sar1->header, sar1->header->file_offs + sar1->stride * file_idx);

	if (file->size == 0)
	{
		return NULL;
	}

	return MEMOFFS(sar1->header, file->offs);
}

const void* sar1_file_meta(sar1_t* sar1, uint32 file_idx)
{
	if (file_idx >= sar1->header->file_num)
	{
		return NULL;
	}

	const sar1_file_t* file = MEMOFFS(sar1->header, sar1->header->file_offs + sar1->stride * file_idx);

	return MEMOFFS(file, sizeof(sar1_file_t));
}

uint32 sar1_file_count(sar1_t* sar1)
{
	return sar1->header->file_num;
}
