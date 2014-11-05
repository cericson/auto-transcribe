#include <stdio.h>
#include <stdlib.h>
#include "piano.h"


// Renders a song into an actual audio signal
// Assumes all piano audio files are exactly 10 seconds and the note starts at 1 second
void render_music(song* s, int** signal, wav_info* header)
{
	note n;
	int i, j, sig, siglen, offset;
	
	siglen = get_data_len(header);
	*signal = malloc(siglen*sizeof(int));
	// Initialize signal as silence
	for (i=0; i<siglen; i++)
	{
		(*signal)[i] = 0;
	}
	
	// Add each note
	for (i=0; i < s->size; i++)
	{
		n = s->notes[i];
		// Add each sample of the note
		for (j=0; (j<(n.dur+FS) && j<(FS*10) && (j+n.start)<siglen); j++)
		{
			offset = j + n.start - FS;
			if (offset >= 0)
			{
				// Scale note volume
				sig = (piano_notes[n.pitch][j]*(n.volume+1))>>8;
				// Superimpose note onto signal
				(*signal)[offset] = (*signal)[offset] + sig;
			}
		}
	}
}

// Loads the piano sound files 0.wav to 87.wav from the ./notes folder 
int init_piano()
{
	int i;
	char filename[16];
	FILE* fp;
	wav_info header;
	
	piano_notes = malloc(PIANO_KEYS*sizeof(int*));
	for (i=0; i<PIANO_KEYS; i++)
	{
		// Generate filename and open file
		sprintf(filename, "notes/%d.wav", i);
		if (open_wav_r(filename,&fp)<0 || check_wav_header(fp,&header)<0)
		{
			printf("Error opening %s\n",filename);
			return -1;
		}
		
		read_signal(fp,&header,&piano_notes[i]); // Read note signal into piano_notes
		
		fclose(fp);
	}
	return 0;
}

// Frees all the memory associated with the piano note signals
int dest_piano()
{
	int i;
	
	for (i=0; i<PIANO_KEYS; i++)
	{
		free(piano_notes[i]); // First free memory for each of the notes
	}
	free(piano_notes); // Lastly free the array memory
	
	return 0;
}