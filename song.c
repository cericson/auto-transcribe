#include <stdio.h>
#include <stdlib.h>
#include "song.h"
#include "file_rw.h"
//#include "wav_rw.h"


// Sets up a new song with base capacity 10
void init_song(song* s)
{
	s->capacity = 10;
	s->size = 0;
	s->notes = malloc(s->capacity*sizeof(note));
	s->fitness = 0;
}

// Adds note to song and increases size of notes array if necessary
void add_note(note n, song* s)
{
	int i;
	note* new;
	// If new note will not fit into notes array
	if (s->size >= s->capacity)
	{
		s->capacity <<= 1; // Double capacity
		new = malloc(s->capacity*sizeof(note)); // Allocate more space
		// Move notes from old array into new
		for (i=0; i < s->size; i++)
		{
			new[i] = s->notes[i];
		}
		free(s->notes); // Clear old array
		s->notes = new; // Set to new array
	}
	s->notes[s->size] = n;	// Add note to array
	s->size++;				// Increase size by 1
}

// Removes a note from a song
void remove_note(song* s, int idx)
{
	int i;

	if (idx >=0 && idx < s->size)
	{
		// Shift all notes in the array down by 1 starting just after the index to
		// be removed, clearing the specified index without a gap
		for (i=idx; i < (s->size-1); i++)
		{
			s->notes[i] = s->notes[i+1];
		}
		
		s->size--; // Decrease size by 1
	}
}

// Performs necessary memory freeing to destroy a song
void dest_song(song* s)
{
	free(s->notes);
}


// Saves a song to a text file
void write_song(song* s, char* filename)
{
	FILE* fp;
	int i;
	fp = fopen(filename, "wb");
	
	// Data for entire song: parents, fitness, number of notes
	fprintf(fp, "Parents: %d, %d\r\nFitness: %E\r\nSize: %d\r\n", 
			s->parent1, s->parent2, s->fitness, s->size);
	
	// Data for each note
	for (i=0; i < s->size; i++)
	{
		fprintf(fp, "[pitch=%d, start=%.3f, dur = %.3f, volume=%d]\r\n",
				s->notes[i].pitch,
				((double)s->notes[i].start)/FS,
				((double)s->notes[i].dur)/FS,
				s->notes[i].volume);
	}
	
	fclose(fp);
}

