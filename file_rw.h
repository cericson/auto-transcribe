#ifndef FILE_RW
#define FILE_RW

void write_int32(int val, FILE* fp);
void write_int16(int val, FILE* fp);
void writeZeros(int num, FILE* fp);
int get_int32(FILE* fp);
int get_int16(FILE* fp);

#endif