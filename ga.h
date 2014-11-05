#ifndef GA
#define GA

#include "song.h"

void gen_pop(song* pop, int pop_size, int est_notes, int upper_lim);
void mutate_pop(song** pop, int pop_size, int upper_lim);
void splice(song* in1, song* in2, song* out1, song* out2);
void mutate_note(note* n, int upper_lim);
void randomize_note(note* n, int upper_lim);
double initial_err(double* goal, int tsize);
double error_fn(double* tform, double* goal, int tsize);

#endif