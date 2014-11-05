#ifndef PIANO
#define PIANO

#include "song.h"
#include "wav_rw.h"
#define PIANO_KEYS 88	// Number of piano keys

// Keeps track of all the signals used for each note of the piano
int** piano_notes;

void render_music(song* s, int** signal, wav_info* header);
int init_piano();
int dest_piano();

#endif