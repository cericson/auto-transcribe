// at.c
// Author: Carl Ericson

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "file_rw.h"
#include "wav_rw.h"
#include "bmp_write.h"
#include "song.h"
#include "piano.h"
#include "ga.h"

#define PI 3.14159265358979
#define baseF 27.5		// Lowest frequency on piano
#define W_KEYS 112		// Upper range of wavelet transform (larger than PIANO_KEYS so harmonics
						// of higher notes are accounted for)
#define NOTES_EST 15	// Estimated number of notes in the song
#define POP_SIZE 100	// Population size
#define NUM_GEN 100		// Number of generations

// Information for the wavelet transform
typedef struct process_info
{
	int height; // Output transform height
	int width;	// Output transform width
	double st;	// Start time of transform relative to start of audio sample
	double et;	// End time of transform relative to start of audio sample
	double b1;	// Beta value of transform
	double b2;	// Second beta value -- currently unsupported
	int sqrtt;	// Square root option with multiple beta values -- currently unsupported
	int phase;	// Whether to calculate phase
	int us;		// Whether to speed up calculation time by undersampling
} process_info;


int check_inputs(int argc, char* argv[], process_info* pi);
void print_arr(double arr[], int s);
double conv(double arr[], int s, int* sig, int datalen, int i);
int wavelet_trans(wav_info* header, int datalen, process_info* pi,
		int* signal, double* tform, double* tphase);
void normalize_transform(double* tform, int t_size, double max);
void writeToImage(char* filename, process_info* pi, double* tform, double* tphase);


int main(int argc, char* argv[])
{
	puts("Auto-transcribe v0.1");
	
	int datalen, t_size;
	FILE* wav_fp;
	wav_info header;
	double *transform, *transphase;
	int* signal;

	// Set defaults for the wavelet transform settings
	process_info p_i = { .sqrtt = 0, .b1 = 16, .b2 = 1, .st = 0,
	.et = 15, .width = 1000, .height = W_KEYS, .phase = 0, .us = 0 };
	
	t_size = p_i.height*p_i.width; // Number of data points in transform
	
	// Check inputs and return usage message if necessary
	if (check_inputs(argc, argv, &p_i) < 0)
	{
		printf("Usage: at [-w width] [-h height] [-st start time] "
		"[-et end time] [-b b1] [-b2 b2] [-s] [-p] [-us]\n <in.wav> <out.bmp>");
		return 0;
	}

	// Attempt to open and check validity of input wav file
	if (open_wav_r(argv[argc-2],&wav_fp)<0 || check_wav_header(wav_fp,&header)<0)
	{
		return 1;
	}
	
	// How long the input data is in samples
	datalen = get_data_len(&header);
	
	transform = malloc(t_size*sizeof(double)); // The wavelet transform (magnitude) of the signal
	transphase = malloc(t_size*sizeof(double));		// The transform phase

	puts("Reading input...");
	read_signal(wav_fp,&header,&signal); // Read input signal into array
	// Wavelet transform on input
	puts("Transforming input...");
	wavelet_trans(&header, datalen, &p_i, signal, transform, transphase);
	// Save output image of input
	puts("Writing output...");
	writeToImage(argv[argc-1], &p_i, transform, transphase);
	
	
	// Clean up and free memory
	free(signal);	
	free(transform);
	free(transphase);
	fclose(wav_fp);

	return 0;
}

// Checks the command line inputs to the program
int check_inputs(int argc, char* argv[], process_info* pi)
{
	int i;
	if (argc<3)
	{
		printf("Not enough arguments:\n");
		return -1;
	}
	for (i=1; i<(argc-2); i++)
	{
		if (strcmp(argv[i],"-w")==0)
		{
			if (i>=(argc-3)) // User used -w, did not specify width value
			{
				printf("Width not specified:\n");
				return -1;
			}
			i++;
			pi->width = atoi(argv[i]);
		}
		if (strcmp(argv[i],"-h")==0)
		{
			if (i>=(argc-3)) // User used -h, did not specify height value
			{
				printf("Height not specified:\n");
				return -1;
			}
			i++;
			pi->height = atoi(argv[i]);
		}
		if (strcmp(argv[i],"-b")==0)
		{
			if (i>=(argc-3)) // User used -b, did not specify transform value
			{
				printf("Beta value 1 not specified:\n");
				return -1;
			}
			i++;
			pi->b1 = atof(argv[i]);
		}
		if (strcmp(argv[i],"-b2")==0)
		{
			if (i>=(argc-3)) // User used -b2, did not specify transform value
			{
				printf("Beta value 2 not specified:\n");
				return -1;
			}
			i++;
			pi->b1 = atof(argv[i]);
		}
		if (strcmp(argv[i],"-st")==0)
		{
			if (i>=(argc-3)) // User used -st, did not specify start time
			{
				printf("Start time not specified:\n");
				return -1;
			}
			i++;
			pi->st = atof(argv[i]);
		}
		if (strcmp(argv[i],"-et")==0)
		{
			if (i>=(argc-3)) // User used -et, did not specify end time
			{
				printf("End time not specified:\n");
				return -1;
			}
			i++;
			pi->et = atof(argv[i]);
		}
		if (strcmp(argv[i],"-s")==0)
		{
			pi->sqrtt = 1;
		}
		if (strcmp(argv[i],"-p")==0)
		{
			pi->phase = 1;
		}
		if (strcmp(argv[i],"-us")==0)
		{
			pi->us = 1;
		}
	}
	
	printf("Generating a %dx%d image with beta=%g.\n",pi->width,pi->height,pi->b1);
	
	return 0;
}

// Prints an array of doubles to MATLAB format: used for debugging
void print_arr(double arr[], int s)
{
	int i;
	printf("[");
	for (i=0; i<s; i++)
	{
		printf("%f;\n",arr[i]);
	}
	printf("]\n");
}

// Evaluate convolution of wavelet arr (of length s) with signal sig (of length datalen)
// centered at sample i of signal
double conv(double arr[], int s, int* sig, int datalen, int i)
{
	int j, elnum;
	double sum = 0;
	for (j=0; j<s; j++)
	{
		elnum = i-(s>>1)+j; // elnum is centered around 0
		if (elnum>=0 && elnum<datalen) // Prevent exceeding bounds of sig array
		{
			sum += (arr[j]*sig[elnum]);
		}
	}

	return sum;
}

// Performs wavelet transform
int wavelet_trans(wav_info* header, int datalen, process_info* pi,
		int* signal, double* tform, double* tphase)
{
	int x, y, i, last_eval_i, s_frac;
	double T, b, s, A, r, j, max=0, timelen;
	int N, mid;
	
	double *w_r, *w_j;
	
	// Change start/end times if invalid
	timelen = ((double)datalen)/header->sample_rate;
	if (pi->st < 0) pi->st = 0;
	if (pi->et > timelen)
	{
		pi->et = timelen;
		//printf("Changed end time to %g seconds.\n", pi->et);
	}

	b = pi->b1;
	for (y=0; y<pi->height; y++)
	{
		// Period of wavelet: set up so the highest frequency is at the top of image
		// and the lowest frequency is at the bottom
		T = header->sample_rate/
				(baseF*pow(2.0,((double)y)/pi->height*W_KEYS/12));
		// Std deviation of gaussian envelope: proportional to period
		s = T*b;
		// Undersampling rate: no need to evaluate at points much closer together
		// than the std deviation 
		s_frac = (int)s>>1;
		// Wavelet amplitude: 1/s negates the convolution value being proportional
		// to s
		A = 1/s;
		// Length in samples of wavelet
		N = ((int)s)*8 + 1;
		// Midpoint of wavelet in samples
		mid = (N-1)/2;

		// Calculate real and imaginary wavelet values
		w_r = malloc(N*sizeof(double));
		w_j = malloc(N*sizeof(double));	
		for (i=0; i<N; i++)
		{
			w_r[i] = A*exp(-((long long)(i-mid))*(i-mid)/(s*s))*cos(2*PI/T*(i-mid));
			w_j[i] = A*exp(-((long long)(i-mid))*(i-mid)/(s*s))*sin(2*PI/T*(i-mid));
		}
		
		
		
		// Large negative initial value for last evaluated sample so the first
		// will always be evaluated
		last_eval_i = -header->sample_rate*20;

		for (x=0; x<pi->width; x++)
		{
			if (y == 114)
			{
				printf("x = %d\n", x);
			}
			// Sample number to evaluate convolution at
			i = (x*(pi->et-pi->st)/pi->width+pi->st)*header->sample_rate;
			// If undersampling is specified, only recalculate convolution values
			// if the last evaluated sample number was more than s_frac samples ago
			if (((i-last_eval_i) > s_frac) || !pi->us)
			{
				// Real and imaginary components
				r = conv(w_r, N, signal, datalen, i);
				j = conv(w_j, N, signal, datalen, i);
				
				// Calculate transform magnitude
				tform[y*pi->width+x] = sqrt(r*r+j*j);
				if (tform[y*pi->width+x] > max)
				{
					max = tform[y*pi->width+x];
				}
				// Calculate transform phase angle
				tphase[y*pi->width+x] = atan2(j,r);
				
				last_eval_i = i;
			}
			else // To save calculation time, use previous values if undersampling
			{
				tform[y*pi->width+x] = tform[y*pi->width+x-1];
				tphase[y*pi->width+x] = tphase[y*pi->width+x-1];
			}
		}

		free(w_r);
		free(w_j);
	}
	// Make maximum value of transform 1
	normalize_transform(tform, pi->width*pi->height, max);
	
	return 1;
}

// Normalizes the transform values so the maximum is 1 by dividing all by the maximum
void normalize_transform(double* tform, int t_size, double max)
{
	int i;
	if (max!=0)
	{
		for (i=0; i<t_size; i++)
		{
			tform[i] /= max;
		}
	}
}

// Write transform to image
void writeToImage(char* filename, process_info* pi, double* tform, double* tphase)
{
	FILE* gen_bmp;
	int x, y;
	struct HSL hsl;
	struct RGB rgb;
	
	// Open output bmp file
	gen_bmp = fopen(filename,"w");
	if (gen_bmp<0)
	{
		perror("Error opening output file"); 
		exit(errno);
	}
	// Write file header
	write_bmp_header(gen_bmp, pi->height, pi->width);
	
	// Write scaled values to image
	hsl.S = 1;
	
	// Write pixels
	for (y=0;y<pi->height;y++)
	{
		for (x=0;x<pi->width;x++)
		{
			// If phase coloring specified
			if (pi->phase)
			{
				// Turn phase into a hue
				hsl.H = (tphase[y*pi->width+x]+PI)/(2*PI);
				// Brightness is proportional to the transform magnitude
				hsl.L = tform[y*pi->width+x]/2;
				// Convert hue, saturation, and lum to RGB
				toRGB(&hsl,&rgb);
			}
			else
			{
				// Grayscale with brightness proportional to the transform magnitude
				rgb.R = rgb.G = rgb.B = (float)(tform[y*pi->width+x]);
			}
			// Write pixel
			write_color(&rgb,gen_bmp);
		}
		// Write bitmap line padding
		writeZeros(pi->width%4, gen_bmp);
	}
	fclose(gen_bmp);
}

