#include "file.h"

void* file_read(const char* path, size_t* out_size)
{
	FILE* file;

	if (fopen_s(&file, path, "rb"))
	{
		return NULL;
	}

	fseek(file, 0, SEEK_END);

	size_t size = ftell(file);

	fseek(file, 0, SEEK_SET);

	void* buffer = malloc(size);

	fread(buffer, 1, size, file);

	if (out_size)
	{
		*out_size = size;
	}

	fclose(file);

	return buffer;
}

void file_write(const char* path, void* data, size_t size)
{
	FILE* file;

	if (fopen_s(&file, path, "wb"))
	{
		return;
	}

	fwrite(data, 1, size, file);
	fclose(file);
}
