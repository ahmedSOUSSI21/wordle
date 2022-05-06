CC = gcc
CFLAGS = -Wall -Wfatal-errors -g
LDFLAGS = -pthread
OBJECTS = mots_5_lettres.o wordle_serveur.o
OBJCLT = wordle_client.o 
PROGS = wordleServeur wordleClient

all: $(PROGS)

wordleServeur: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)
wordleClient: $(OBJCLT)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJCLT)

mots_5_lettres.o: mots_5_lettres.c mots_5_lettres.h
wordle_serveur.o: wordle_serveur.c mots_5_lettres.h
wordle_client.o : wordle_client.c

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(PROGS)
