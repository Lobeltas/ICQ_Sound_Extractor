#include <windows.h>
#include <stdio.h>
#include <sys/types.h> 
#include <cstdint>

int readUInt32(unsigned char* _Ptr, uint32_t* _Dest)
{
	*_Dest = 0;
	*_Dest = (uint32_t)((unsigned char*)_Ptr)[3]; *_Dest <<= 8;
	*_Dest |= (uint32_t)((unsigned char*)_Ptr)[2]; *_Dest <<= 8;
	*_Dest |= (uint32_t)((unsigned char*)_Ptr)[1]; *_Dest <<= 8;
	*_Dest |= (uint32_t)((unsigned char*)_Ptr)[0];
	return 4;
}


int readHeaderFile(unsigned char* _Ptr, const char** _Filename, uint32_t* _Size)
{
	const char* filename = (const char*)_Ptr;
	int length = strlen(filename);
	_Ptr += length + 1;

	uint32_t size;
	_Ptr += readUInt32(_Ptr, &size);

	*(_Filename) = filename;
	*(_Size) = size;
	return length + 5;
}

int extractFile(unsigned char* _Buffer, const char* _Filename, int _Address, int _Size)
{
	FILE* file = NULL;
	fopen_s(&file, _Filename, "w+b");
	if (file == NULL)
	{
		printf("Failed to open file %sd for write!", _Filename);
		return -1;
	}

	unsigned char* ptr = &_Buffer[_Address];
	int bytesLeft = _Size;
	int bytesWritten = 0;
	while (bytesLeft > 0)
	{
		bytesWritten = fwrite(ptr, 1, bytesLeft, file);
		if (bytesWritten < 0)
		{
			printf("Failed to write to file!");
			return -2;
		}

		bytesLeft -= bytesWritten;
		ptr += bytesWritten;
	}

	fclose(file);
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: ICQ_Sound_Extractor.exe filename.scm");
		return 1;
	}
	
	printf("Extracting sounds from %s\r\n", argv[1]);

	FILE* file = NULL;
	fopen_s(&file, argv[1], "r+b");
	if (file == NULL)
	{
		printf("Failed to open file!");
		return -1;
	}
		

	fseek(file, 0, SEEK_END);
	int totalSize = ftell(file);

	fpos_t position;
	position = 0;
	fsetpos(file, &position);


	unsigned char* buffer = (unsigned char*)malloc(totalSize);
	if (buffer == NULL)
	{
		printf("Failed to alloc memory!");
		return -2;
	}

	unsigned char* ptr = buffer;
	size_t bytesLeft = totalSize;
	size_t bytesRead = 0;
	while (bytesLeft > 0)
	{
		bytesRead = fread(ptr, 1, bytesLeft, file);
		if (bytesRead < 0)
		{
			printf("Failed to read file!\r\n");
			return -3;
		}
		ptr += bytesRead;
		bytesLeft -= bytesRead;
	}

	ptr = buffer;
	uint32_t headerSize;
	
	ptr += readUInt32(ptr, &headerSize);
	printf("HeaderSize: %u\r\n\r\nSearching for files...\r\n\r\n", headerSize);

	int currentAddress = headerSize + 4;
	while (ptr - buffer < headerSize)
	{
		const char* filename = NULL;
		uint32_t size = 0;
		ptr += readHeaderFile(ptr, &filename, &size);
		if (size > 0)
		{
			printf("Extracting file \"%s\" (Filesize = %u)...", filename, size);
			if (extractFile(buffer, filename, currentAddress, size) == 0)
				printf("OK\r\n");
			else
				printf("FAILED!\r\n");

			currentAddress += size;
		}
	}

	printf("DONE!\r\n");

	free(buffer);

	fclose(file);

	return 0;
}
