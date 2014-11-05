#ifndef SONG
#define SONG

#define FS 44100		// Sample rate

// Basic note: subunit of a song
typedef struct note
{
	unsigned int pitch;		// Ranges from 0 to 87 (88 keys on a piano)
	unsigned int start;		// Start time in samples
	unsigned int dur;		// End time in samples
	unsigned int volume;	// ranges from 0-255
} note;

// Songs: the individuals for the genetic algorithm
typedef struct song
{
	note* notes;	// List of notes in song
	int size;		// Number of notes in song
	int capacity;	// Capacity of notes array: can increase if necessary
	double fitness;	// Fitness of individual
	int parent1;	// Parent numbers of individual for later reference
	int parent2;
} song;

void init_song(song* s);
void add_note(note n, song* s);
void remove_note(song* s, int idx);
void dest_song(song* s);
void write_song(song* s, char* filename);


#endif