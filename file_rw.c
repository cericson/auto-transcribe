#include <stdio.h>
#include "file_rw.h"

//writes a 32 bit int to a file
void write_int32(int val, FILE* fp)
{
	fputc(val&0xFF,fp);
	fputc((val>>8)&0xFF,fp);
	fputc((val>>16)&0xFF,fp);
	fputc((val>>24)&0xFF,fp);
}

//writes a 16 bit int to a file
void write_int16(int val, FILE* fp)
{
	fputc(val&0xFF,fp);
	fputc((val>>8)&0xFF,fp);
}

//writes the given number of null characters to a file
void writeZeros(int num, FILE* fp)
{
	int k;
	for (k=0;k<num;k++)
	{
		fputc(0,fp);
	}
}

// Gets four bytes and returns it as a 32 bit integer value (little endian)
int get_int32(FILE* fp)
{
	return (fgetc(fp)|(fgetc(fp)<<8)|(fgetc(fp)<<16)|(fgetc(fp)<<24));
}

// Gets two bytes and returns it as a 16 bit integer value (little endian)
int get_int16(FILE* fp)
{
	return (fgetc(fp)|(fgetc(fp)<<8));
}