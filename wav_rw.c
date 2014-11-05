#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav_rw.h"
#include "file_rw.h"

// Opens wav file for reading
int open_wav_r(char* filename, FILE** fp)
{
	int filenamelen;
	char* fileext;
	
	filenamelen = strlen(filename);		// Filename string length
	fileext = &filename[filenamelen-4]; 	// Get last 4 characters of filename

	// Check for valid file extension
	if (strcmp(fileext,".wav")!=0)
	{
		printf("Error: Input file must be a .wav file.\n");
		return -1;
	}

	*fp = fopen(filename,"rb");
	if (*fp==NULL)
	{
		perror(filename);
		return -1;
	}
	
	return 0;
}

// Writes a signal to a wav file (mono only)
int write_wav(char* filename, wav_info* p_header, int* signal)
{
	int i, siglen;
	FILE* fp;
	
	// Open file
	fp = fopen(filename,"w");
	if (fp==NULL)
	{
		perror(filename);
		return -1;
	}
	
	// Write header data to file
	write_wav_header(p_header, fp);
	
	// Write signal to file
	siglen = get_data_len(p_header);
	for (i=0; i<siglen; i++)
	{
		write_sample(signal[i], p_header, fp);
	}
	
	fclose(fp);
	
	return 0;
}

// Writes the header data to a wav file
void write_wav_header(wav_info* p_header, FILE* m_fp)
{
	fputs("RIFF", m_fp);
	write_int32(p_header->chunk_size, m_fp);
	fputs("WAVEfmt ", m_fp);
	write_int32(p_header->subchunk1_size, m_fp);
	write_int16(p_header->audio_format, m_fp);
	write_int16(p_header->n_channels, m_fp);
	write_int32(p_header->sample_rate, m_fp);
	write_int32(p_header->byte_rate, m_fp);
	write_int16(p_header->block_align, m_fp);
	write_int16(p_header->bits_per_sample, m_fp);
	fputs("data", m_fp);
	write_int32(p_header->subchunk2_size, m_fp);
}

// Write a single sample of an audio signal to file
void write_sample(int sample, wav_info* header, FILE* fp)
{
	if (header->bits_per_sample == 8)
	{
		fputc(sample, fp);
	}
	else if (header->bits_per_sample == 32)
	{
		write_int32(sample, fp);
	}
	else
	{
		write_int16(sample, fp);
	}
}

// Adjusts header data to make header information mono (1-channel)
void make_mono(wav_info* p_header)
{
	// Values proportional to number of channels 
	p_header->block_align /= p_header->n_channels;
	p_header->byte_rate /= p_header->n_channels;
	p_header->subchunk2_size /= p_header->n_channels;
	p_header->chunk_size = (p_header->chunk_size-36)/p_header->n_channels+36;
	p_header->n_channels = 1; // Actual number of channels
}

// Checks for a valid wav file header
// wav file information from https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
int check_wav_header(FILE* m_fp, wav_info* p_header)
{
	int bps;
	char temp[9];
	if (fgets(temp,5,m_fp)==NULL)
	{
		perror("Read error");
		return -1;
	}

	if (strcmp(temp,"RIFF")!=0)
	{
		printf("Invalid file header.\n");
		return -1;
	}
	
	p_header->chunk_size = get_int32(m_fp);

	if (fgets(temp,9,m_fp)==NULL)
    {
        perror("Read error");
        return -1;
    }

    if (strcmp(temp,"WAVEfmt ")!=0)
    {
        printf("Invalid file header.\n");
		return -1;
    }

	p_header->subchunk1_size = get_int32(m_fp);

	if (p_header->subchunk1_size!=16) // must be 16 for uncompressed format
	{
		printf("Compressed format unsupported.\n");
		return -1;
	}
	
	p_header->audio_format = get_int16(m_fp);

	if (p_header->audio_format != 1) // if data is compressed
	{
		printf("Compressed format unsupported.\n");
		return -1;
	}
	p_header->n_channels = get_int16(m_fp);
	p_header->sample_rate = get_int32(m_fp);
	p_header->byte_rate = get_int32(m_fp);
	p_header->block_align = get_int16(m_fp);
	p_header->bits_per_sample = get_int16(m_fp);
	bps = p_header->bits_per_sample;
	if (bps!=8 && bps!=16 && bps!=32)
	{
		printf("Invalid resolution in file header.\n");
        return -1;
	}
	
	if(p_header->bits_per_sample*p_header->n_channels/8!=p_header->block_align)
	{
		printf("Invalid value in file header.\n");
		return -1;
	}
	if (p_header->sample_rate*p_header->block_align != p_header->byte_rate)
	{
		printf("Invalid value in file header.\n");
		return -1;
	}

	if (fgets(temp,5,m_fp)==NULL)
    {
        perror("Read error");
        return -1;
    }

    if (strcmp(temp,"data")!=0)
    {
        printf("Invalid file header.\n");
        return -1;
    }

	p_header->subchunk2_size = get_int32(m_fp);

	if (36 + p_header->subchunk2_size != p_header->chunk_size)
	{
		printf("Invalid value in file header.\n");
        return -1;
	}
	
	//printf("File validity confirmed.\n");
	
	return 0;
}

// Reads a single sample from a wav file
int get_sample(FILE* fp, wav_info* header)
{
	int temp;
	if (header->bits_per_sample == 8)
	{
		temp = fgetc(fp);
		if (temp & 0x80)
		{
			temp |= 0xFFFFFF00;
		}
		return temp;
	}
	else if (header->bits_per_sample == 32)
	{
		return get_int32(fp);
	}
	else
	{
		temp = get_int16(fp);
		if (temp & 0x8000)
		{
			temp |= 0xFFFF0000;
		}
		return temp;
	}
}

// Calculates the length of an audio file in samples from the header data
int get_data_len(wav_info* header)
{
	return header->subchunk2_size/((header->bits_per_sample>>3)*header->n_channels);
}

// Reads signal from file into array (automatically allocates memory for array from header data)
// Note: converts signal to mono by adding samples on multiple channels
void read_signal(FILE* fp, wav_info* header, int** p_signal)
{
	int i, k, datalen;
	
	datalen = get_data_len(header);
	(*p_signal) = malloc(datalen*sizeof(int));
	
	fseek(fp, WAV_DATA_OFFSET, SEEK_SET);
	
	// Read each sample
	for (i=0; i<datalen; i++)
	{
		(*p_signal)[i] = 0;
		// Sum up samples of each channel
		for (k=0; k<header->n_channels; k++)
		{
			(*p_signal)[i] += get_sample(fp,header);
		}
	}
}