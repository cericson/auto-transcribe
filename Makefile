CC=gcc
CFLAGS=-O3 -g -Wall -lm
OBJS=at.o file_rw.o wav_rw.o bmp_write.o song.o piano.o ga.o 
EXE=at

at : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE)

at.o : at.c
	$(CC) $(CFLAGS) -c at.c

file_rw.o : file_rw.c file_rw.h
	$(CC) $(CFLAGS) -c file_rw.c

wav_rw.o : wav_rw.c wav_rw.h file_rw.o
	$(CC) $(CFLAGS) -c wav_rw.c

bmp_write.o : bmp_write.c bmp_write.h file_rw.o
	$(CC) $(CFLAGS) -c bmp_write.c
	
piano.o : piano.c piano.h song.o wav_rw.o
	$(CC) $(CFLAGS) -c piano.c
	
song.o : song.c song.h file_rw.o
	$(CC) $(CFLAGS) -c song.c
	
ga.o : bmp_write.c bmp_write.h song.o piano.o
	$(CC) $(CFLAGS) -c ga.c

clean :
	rm $(OBJS) $(EXE)

cleanout :
	rm individuals/*/*.bmp
	rm individuals/*/*.wav
	rm individuals/*/*.txt
