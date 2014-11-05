#include <stdlib.h>
#include "ga.h"
#include "piano.h"

// Generates the population of songs
void gen_pop(song* pop, int pop_size, int est_notes, int upper_lim)
{
	int i, j;
	note n;
	for (i=0; i<pop_size; i++)
	{
		init_song(&pop[i]); // Create song
		// Add est_notes random notes to the song
		for (j=0; j<est_notes; j++)
		{
			randomize_note(&n, upper_lim);
			add_note(n,&pop[i]);
		}
	}
}

// Mutates the population (upper_lim is the maximum sample number)
void mutate_pop(song** pop, int pop_size, int upper_lim)
{
	int i, j, k, sel, idx;
	double fitsum = 0;
	double rnd;
	int selnum = pop_size/2; // Select half of the population to reproduce
	song** selections = malloc(selnum*sizeof(song*)); // Keeps track of selections
	int* parent = malloc(selnum*sizeof(int)); // Keeps track of parents for reference
	song* newpop = malloc(pop_size*sizeof(song)); // New population
	note n;
	song s1, s2, s3, s4;
	
	// Sum all fitnesses for roulette wheel selection
	for (i=0; i<pop_size; i++)
	{
		fitsum += (*pop)[i].fitness;
	}

	for (i=0; i<selnum; i++)
	{
		// Choose random individual with roulette wheel selection
		rnd = ((double)rand())/RAND_MAX*fitsum;
		sel=0;
		
		// Reverse of the usual method: subtract individual fitnesses until 0 is reached
		rnd -= (*pop)[sel].fitness;
		while (rnd>0)
		{
			sel++;
			rnd -= (*pop)[sel].fitness;
		}
		// Save selection and the individual number
		selections[i] = &(*pop)[sel];
		parent[i] = sel;
	}

	// Selections reproduce in groups of two
	for (i=0; i<selnum; i+=2)
	{
		// Create 4 new individuals by crossing over the note arrays of each twice
		splice(selections[i],selections[i+1],&s1,&s2);
		splice(selections[i],selections[i+1],&s3,&s4);
		newpop[2*i] = s1;
		newpop[2*i+1] = s2;
		newpop[2*i+2] = s3;
		newpop[2*i+3] = s4;
		
		// Mutate each child
		for (j=0; j<4; j++)
		{
			idx = 2*i+j;
			
			// Save parent numbers for each child
			newpop[idx].parent1 = parent[i];
			newpop[idx].parent2 = parent[i+1];
			// Mutate each note of the song
			for (k=0; k < newpop[idx].size; k++)
			{
				mutate_note(&(newpop[idx].notes[k]), upper_lim);
			}
			
			// Randomly add or remove note
			if ((rand()%16) == 0)
			{
				randomize_note(&n, upper_lim);
				add_note(n, &newpop[idx]);
			}
			if ((rand()%16) == 0 && newpop[idx].size!=0)
			{
				remove_note(&newpop[idx], rand()%newpop[idx].size);
			}
		}
	}
	free(selections);
	free(parent);
	
	// Free the memory of the old population
	for (i=0; i<pop_size; i++)
	{
		dest_song(&(*pop)[i]);
	}
	free(*pop);
	*pop = newpop; // Set the population to the new population
}

// Performs a crossover on two parent songs to generate two child songs
void splice(song* in1, song* in2, song* out1, song* out2)
{
	int i, p;
	// p: crossover point
	p = rand()%((in1->size < in2->size)?(in1->size+1):(in2->size+1));
	// Initialize children
	init_song(out1);
	init_song(out2);
	
	// Copy notes from before the crossover point to the children
	for (i=0; i<p; i++)
	{
		add_note(in1->notes[i], out1);
		add_note(in2->notes[i], out2);
	}
	// Copy notes after the crossover point to the children
	for (i=p; i < in2->size; i++)
	{
		add_note(in2->notes[i], out1);
	}
	for (i=p; i < in1->size; i++)
	{
		add_note(in1->notes[i], out2);
	}
}

// Mutates an individual note
void mutate_note(note* n, int upper_lim)
{
	int i, mask, dev_start = 3, dev_dur = 2; // Start time can mutate more than duration
	
	// The second, third, and fourth harmonics are 12, 19, and 24 keys away, respectively, so
	// notes can randomly shift harmonics (since in the wavelet transform, notes may line up
	// with harmonics instead of the other notes)
	if (rand()%8 == 0 && (n->pitch < PIANO_KEYS-12)) n->pitch += 12;
	if (rand()%8 == 0 && (n->pitch >= 12)) n->pitch -= 12;
	if (rand()%16 == 0 && (n->pitch < PIANO_KEYS-19)) n->pitch += 19;
	if (rand()%16 == 0 && (n->pitch >= 19)) n->pitch -= 19;
	if (rand()%32 == 0 && (n->pitch < PIANO_KEYS-24)) n->pitch += 24;
	if (rand()%32 == 0 && (n->pitch >= 24)) n->pitch -= 24;
	if (rand()%16 == 0) n->pitch = rand()%PIANO_KEYS;
	
	// Mutate start time and duration
	for (i=0; i<32; i++)
	{
		// Chance to flip each bit of the start value
		mask = 1<<i;
		// Lower chance of flipping more significant bits
		if (rand()%(4<<(i/dev_start)) == 0)
		{
			n->start ^= mask;
		}
		
		if (rand()%(4<<(i/dev_dur)) == 0)
		{
			n->dur ^= mask;
		}
	}
	// Notes outside the bounds of the input song are useless, so move them back
	if (n->start+n->dur > upper_lim)
	{
		n->start = upper_lim - n->dur;
	}
	
	// Mutate volume
	for (i=0; i<8; i++)
	{
		mask = 1<<i;
		if (rand()%(4<<(i)) == 0)
		{
			n->volume ^= mask;
		}
	}
}

// Initializes a new random note
void randomize_note(note* n, int upper_lim)
{
	n->pitch = rand()%PIANO_KEYS;	// Random piano key
	n->start = rand()%upper_lim;	// Start time will not exceed upper limit
	n->dur = rand()%FS; 			// Random duration under a second
	n->volume = rand()%256;			// Random 8 bit volume value
}

// Calculates initial error value (error from silence)
double initial_err(double* goal, int tsize)
{
	int i;
	double total = 0;
	for (i=0; i<tsize; i++)
	{
		total += goal[i]*goal[i];
	}
	return total;
}

// Calculates a total error value (sum of squared individual errors)
double error_fn(double* tform, double* goal, int tsize)
{
	int i;
	double total = 0;
	for (i=0; i<tsize; i++)
	{
		total += (tform[i]-goal[i])*(tform[i]-goal[i]);
	}
	return total;
}
