#ifndef WAV_RW
#define WAV_RW

#include <stdio.h>

#define WAV_DATA_OFFSET 44

typedef struct wav_info
{
	int chunk_size;
	int subchunk1_size;
	int audio_format;
	int n_channels;
	int sample_rate;
	int byte_rate;
	int block_align;
	int bits_per_sample;
	int subchunk2_size;
} wav_info;

int open_wav_r(char* filename, FILE** fp);
int write_wav(char* filename, wav_info* header, int* signal);
void write_wav_header(wav_info* p_header, FILE* m_fp);
void write_sample(int sample, wav_info* header, FILE* fp);
void make_mono(wav_info* p_header);
int check_wav_header(FILE* m_fp, wav_info* p_header);
int get_sample(FILE* fp, wav_info* header);
int get_data_len(wav_info* header);
void read_signal(FILE* fp, wav_info* header, int** signal);

#endif
