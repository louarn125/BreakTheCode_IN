CC     = gcc
CFLAGS = -Wall -Wextra -g -O2

# CAP = capacité de poids du vaisseau, BEAM = largeur du faisceau
CAP  ?= 100
BEAM ?= 3000

.PHONY: all run clean

all: graphs

graphs: graphs.c
	$(CC) $(CFLAGS) graphs.c -o $@

run: graphs
	./graphs $(CAP) $(BEAM)

clean:
	rm -f graphs
