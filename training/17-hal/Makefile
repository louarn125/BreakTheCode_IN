CC     = gcc
CFLAGS = -Wall -Wextra -g -I../..

# Code partagé à la racine du dépôt
COMMON = ../../charge_fichier.c ../../fichier.h

.PHONY: all run clean

all: halcode

halcode: halcode.c $(COMMON)
	$(CC) $(CFLAGS) halcode.c ../../charge_fichier.c -o $@

run: halcode
	./halcode

clean:
	rm -f halcode
